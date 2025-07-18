#include "debugfs_handler.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>

DebugfsHandler::DebugfsHandler() {
    std::cout << "[DEBUG] DebugfsHandler: Constructor called - Clearing dmesg buffer" << std::endl;
    // Clear dmesg buffer at initialization
    clearDmesg();
}

void DebugfsHandler::writeToNode(const std::string& node, int value) {
    std::cout << "[DEBUG] writeToNode: Writing value " << value << " to node " << node << std::endl;
    
    if (!isValidNodeValue(node, value)) {
        std::cout << "[DEBUG] writeToNode: ERROR - Invalid value " << value << " for node " << node << std::endl;
        throw std::invalid_argument("Invalid value for node: " + node);
    }

    std::string nodePath = getNodePath(node);
    std::cout << "[DEBUG] writeToNode: Node path resolved to: " << nodePath << std::endl;
    
    std::ofstream nodeFile(nodePath);
    if (!nodeFile.is_open()) {
        std::cout << "[DEBUG] writeToNode: ERROR - Failed to open node path: " << nodePath << std::endl;
        throw std::runtime_error("Failed to open node: " + nodePath);
    }

    nodeFile << value;
    nodeFile.close();
    std::cout << "[DEBUG] writeToNode: Successfully wrote to node and closed file" << std::endl;

    // Wait a bit for dmesg to be updated
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[DEBUG] writeToNode: Completed 100ms sleep for dmesg update" << std::endl;
}

void DebugfsHandler::captureBaselineStatus() {
    std::cout << "[DEBUG] captureBaselineStatus: Starting baseline capture" << std::endl;
    
    // Clear dmesg before capturing baseline
    clearDmesg();
    std::cout << "[DEBUG] captureBaselineStatus: dmesg cleared before baseline capture" << std::endl;
    
    // Write 0 to case node to get status
    writeToNode(NODE_CASE, 0);
    std::cout << "[DEBUG] captureBaselineStatus: Written 0 to case node" << std::endl;
    
    // Get and store the baseline status
    baselineStatus = getDmesgOutput("msm_pcie");
    std::cout << "[DEBUG] captureBaselineStatus: baselineStatus captured:" << std::endl;
    std::cout << "=== BASELINE STATUS START ===" << std::endl;
    std::cout << baselineStatus << std::endl;
    std::cout << "=== BASELINE STATUS END ===" << std::endl;
    std::cout << "[DEBUG] captureBaselineStatus: Baseline status length: " << baselineStatus.length() << " characters" << std::endl;
}

void DebugfsHandler::saveStatus(const std::string& filename) {
    std::cout << "[DEBUG] saveStatus: Starting status save to file: " << filename << std::endl;
    
    // Write 0 to case node to get current status
    writeToNode(NODE_CASE, 0);
    
    std::string status = getDmesgOutput("msm_pcie");
    std::cout << "[DEBUG] saveStatus: Retrieved status for saving (length: " << status.length() << " characters)" << std::endl;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "[DEBUG] saveStatus: ERROR - Failed to open file for writing: " << filename << std::endl;
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    file << status;
    file.close();
    std::cout << "[DEBUG] saveStatus: Successfully saved status to file: " << filename << std::endl;
}

std::map<std::string, std::string> DebugfsHandler::compareWithBaseline() {
    std::cout << "[DEBUG] compareWithBaseline: Starting comparison with baseline" << std::endl;
    
    // Get current status using case node option 0
    writeToNode(NODE_CASE, 0);
    std::string currentStatus = getDmesgOutput("msm_pcie");
    
    std::cout << "[DEBUG] compareWithBaseline: currentStatus captured:" << std::endl;
    std::cout << "=== CURRENT STATUS START ===" << std::endl;
    std::cout << currentStatus << std::endl;
    std::cout << "=== CURRENT STATUS END ===" << std::endl;
    std::cout << "[DEBUG] compareWithBaseline: Current status length: " << currentStatus.length() << " characters" << std::endl;
    
    auto baselineMap = parseStatus(baselineStatus);
    auto currentMap = parseStatus(currentStatus);
    
    std::cout << "[DEBUG] compareWithBaseline: Parsed baseline map - " << baselineMap.size() << " entries" << std::endl;
    std::cout << "[DEBUG] compareWithBaseline: Parsed current map - " << currentMap.size() << " entries" << std::endl;
    
    std::map<std::string, std::string> differences;
    
    // Find differences between baseline and current status
    for (const auto& current : currentMap) {
        auto baselineIt = baselineMap.find(current.first);
        if (baselineIt == baselineMap.end() || baselineIt->second != current.second) {
            differences[current.first] = current.second;
            std::cout << "[DEBUG] compareWithBaseline: Difference found - Key: '" << current.first 
                      << "' | Current: '" << current.second << "'";
            if (baselineIt != baselineMap.end()) {
                std::cout << " | Baseline: '" << baselineIt->second << "'";
            } else {
                std::cout << " | Baseline: <NOT_FOUND>";
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "[DEBUG] compareWithBaseline: Total differences found: " << differences.size() << std::endl;
    std::cout << "[DEBUG] compareWithBaseline: Differences map keys:" << std::endl;
    for (const auto& diff : differences) {
        std::cout << "  - Key: '" << diff.first << "'" << std::endl;
    }
    
    return differences;
}

std::string DebugfsHandler::getCurrentStatus() {
    std::cout << "[DEBUG] getCurrentStatus: Retrieving current status" << std::endl;
    
    // Get status using case node option 0
    writeToNode(NODE_CASE, 0);
    std::string status = getDmesgOutput("msm_pcie");
    
    std::cout << "[DEBUG] getCurrentStatus: Status retrieved (length: " << status.length() << " characters)" << std::endl;
    return status;
}

std::map<std::string, std::string> DebugfsHandler::parseStatus(const std::string& status) {
    std::cout << "[DEBUG] parseStatus: Starting to parse status string (length: " << status.length() << " characters)" << std::endl;
    
    std::map<std::string, std::string> statusMap;
    std::istringstream stream(status);
    std::string line;
    int lineCount = 0;
    
    while (std::getline(stream, line)) {
        lineCount++;
        std::string key, value;

        // First try to split by "is"
        size_t isPos = line.find(" is ");
        if (isPos != std::string::npos) {
            // Split by "is"
            key = line.substr(0, isPos);
            value = line.substr(isPos + 4); // Skip " is "
            std::cout << "[DEBUG] parseStatus: Line " << lineCount << " parsed with ' is ' separator - Key: '" << key << "' | Value: '" << value << "'" << std::endl;
        } else {
            // If "is" not found, try splitting by ":"
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                key = line.substr(0, colonPos);
                value = line.substr(colonPos + 1);
                std::cout << "[DEBUG] parseStatus: Line " << lineCount << " parsed with ':' separator - Key: '" << key << "' | Value: '" << value << "'" << std::endl;
            } else {
                // If neither separator is found, skip this line
                std::cout << "[DEBUG] parseStatus: Line " << lineCount << " skipped (no valid separator found): '" << line << "'" << std::endl;
                continue;
            }
        }

        // Trim whitespace from key and value
        auto trim = [](std::string& s) {
            // Trim start
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            // Trim end
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };

        trim(key);
        trim(value);
        statusMap[key] = value;
        std::cout << "[DEBUG] parseStatus: After trimming - Key: '" << key << "' | Value: '" << value << "'" << std::endl;
    }
    
    std::cout << "[DEBUG] parseStatus: Parsing completed - Total lines processed: " << lineCount << " | Valid entries: " << statusMap.size() << std::endl;
    return statusMap;
}

std::string DebugfsHandler::getDmesgOutput(const std::string& filter) {
    std::cout << "[DEBUG] getDmesgOutput: Executing dmesg with filter: '" << filter << "'" << std::endl;
    
    std::string cmd = "dmesg";
    if (!filter.empty()) {
        cmd += " | grep \"" + filter + "\"";
    }
    
    std::cout << "[DEBUG] getDmesgOutput: Command: " << cmd << std::endl;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cout << "[DEBUG] getDmesgOutput: ERROR - Failed to execute dmesg command" << std::endl;
        throw std::runtime_error("Failed to execute dmesg command");
    }

    std::string result;
    char buffer[4096];
    size_t bytesRead;
    size_t totalBytesRead = 0;

    // Read in binary mode to handle all characters
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        result.append(buffer, bytesRead);
        totalBytesRead += bytesRead;
    }

    pclose(pipe);
    std::cout << "[DEBUG] getDmesgOutput: Command completed - Total bytes read: " << totalBytesRead << std::endl;
    return result;
}

bool DebugfsHandler::isValidNodeValue(const std::string& node, int value) const {
    bool isValid;
    if (node == NODE_CASE) {
        isValid = value >= 0 && value <= 9;
    } else {
        isValid = value == 0 || value == 1;
    }
    
    std::cout << "[DEBUG] isValidNodeValue: Node '" << node << "' with value " << value << " is " << (isValid ? "VALID" : "INVALID") << std::endl;
    return isValid;
}

std::string DebugfsHandler::getNodePath(const std::string& node) const {
    std::string path = std::string(DEBUGFS_PATH) + node;
    std::cout << "[DEBUG] getNodePath: Node '" << node << "' resolved to path: '" << path << "'" << std::endl;
    return path;
}

void DebugfsHandler::clearDmesg() {
    std::cout << "[DEBUG] clearDmesg: Clearing dmesg buffer" << std::endl;
    system("dmesg -c > /dev/null");
    std::cout << "[DEBUG] clearDmesg: dmesg buffer cleared" << std::endl;
} 