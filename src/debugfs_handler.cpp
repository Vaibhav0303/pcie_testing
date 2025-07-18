#include "debugfs_handler.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <algorithm>

DebugfsHandler::DebugfsHandler() {
    // Clear dmesg buffer at initialization
    clearDmesg();
}

void DebugfsHandler::writeToNode(const std::string& node, int value) {
    if (!isValidNodeValue(node, value)) {
        throw std::invalid_argument("Invalid value for node: " + node);
    }

    std::string nodePath = getNodePath(node);
    std::ofstream nodeFile(nodePath);
    if (!nodeFile.is_open()) {
        throw std::runtime_error("Failed to open node: " + nodePath);
    }

    nodeFile << value;
    nodeFile.close();

    // Wait a bit for dmesg to be updated
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void DebugfsHandler::captureBaselineStatus() {
    // Clear dmesg before capturing baseline
    clearDmesg();
    
    // Write 0 to case node to get status
    writeToNode(NODE_CASE, 0);
    
    // Get and store the baseline status
    baselineStatus = getDmesgOutput("msm_pcie");
}

void DebugfsHandler::saveStatus(const std::string& filename) {
    // Write 0 to case node to get current status
    writeToNode(NODE_CASE, 0);
    
    std::string status = getDmesgOutput("msm_pcie");
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    file << status;
    file.close();
}

std::map<std::string, std::string> DebugfsHandler::compareWithBaseline() {
    // Get current status using case node option 0
    writeToNode(NODE_CASE, 0);
    std::string currentStatus = getDmesgOutput("msm_pcie");
    
    auto baselineMap = parseStatus(baselineStatus);
    auto currentMap = parseStatus(currentStatus);
    
    std::map<std::string, std::string> differences;
    
    // Find differences between baseline and current status
    for (const auto& current : currentMap) {
        auto baselineIt = baselineMap.find(current.first);
        if (baselineIt == baselineMap.end() || baselineIt->second != current.second) {
            differences[current.first] = current.second;
        }
    }
    
    return differences;
}

std::string DebugfsHandler::getCurrentStatus() {
    // Get status using case node option 0
    writeToNode(NODE_CASE, 0);
    return getDmesgOutput("msm_pcie");
}

std::map<std::string, std::string> DebugfsHandler::parseStatus(const std::string& status) {
    std::map<std::string, std::string> statusMap;
    std::istringstream stream(status);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Remove "msm_pcie_show_status:" prefix if it exists
        const std::string prefix = "msm_pcie_show_status:";
        size_t prefixPos = line.find(prefix);
        if (prefixPos != std::string::npos) {
            line = line.substr(prefixPos + prefix.length());
        }
        
        std::string key, value;

        // First try to split by "is"
        size_t isPos = line.find(" is ");
        if (isPos != std::string::npos) {
            // Split by "is"
            key = line.substr(0, isPos);
            value = line.substr(isPos + 4); // Skip " is "
        } else {
            // If "is" not found, try splitting by ":"
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                key = line.substr(0, colonPos);
                value = line.substr(colonPos + 1);
            } else {
                // If neither separator is found, skip this line
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
    }
    
    return statusMap;
}

std::string DebugfsHandler::getDmesgOutput(const std::string& filter) {
    std::string cmd = "dmesg";
    if (!filter.empty()) {
        cmd += " | grep \"" + filter + "\"";
    }

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute dmesg command");
    }

    std::string result;
    char buffer[4096];
    size_t bytesRead;

    // Read in binary mode to handle all characters
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        result.append(buffer, bytesRead);
    }

    pclose(pipe);
    return result;
}

bool DebugfsHandler::isValidNodeValue(const std::string& node, int value) const {
    if (node == NODE_CASE) {
        return value >= 0 && value <= 9;
    } else {
        return value == 0 || value == 1;
    }
}

std::string DebugfsHandler::getNodePath(const std::string& node) const {
    return std::string(DEBUGFS_PATH) + node;
}

void DebugfsHandler::clearDmesg() {
    system("dmesg -c > /dev/null");
} 