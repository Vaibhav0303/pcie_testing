#include "debugfs_handler.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <regex>
#include <array>

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
        // Skip lines that don't contain PCIe status information
        if (line.find("msm_pcie_show_status:") == std::string::npos) {
            continue;
        }
        
        // Extract the part after "msm_pcie_show_status:"
        size_t prefixPos = line.find("msm_pcie_show_status:");
        if (prefixPos == std::string::npos) {
            continue;
        }
        
        std::string content = line.substr(prefixPos + 21); // 21 = length of "msm_pcie_show_status:"
        
        // Trim leading whitespace
        content.erase(0, content.find_first_not_of(" \t"));
        
        if (content.empty()) {
            continue;
        }
        
        std::string key, value;
        
        // Handle different patterns:
        // Pattern 1: "key is value" or "key is value"
        size_t isPos = content.find(" is ");
        if (isPos != std::string::npos) {
            key = content.substr(0, isPos);
            value = content.substr(isPos + 4); // 4 = length of " is "
        }
        // Pattern 2: Look for other common separators like ":"
        else {
            size_t colonPos = content.find(':');
            if (colonPos != std::string::npos) {
                key = content.substr(0, colonPos);
                value = content.substr(colonPos + 1);
            }
            // Pattern 3: If no clear separator, try to split on last space before a value-like token
            else {
                // Look for patterns like "key 0x123", "key 123", "key enable/disable"
                std::regex valuePattern(R"(^(.+?)\s+((?:0x[0-9a-fA-F]+|\d+|enable|disable|enumerated|[A-Z]+\d*))$)");
                std::smatch match;
                if (std::regex_match(content, match, valuePattern)) {
                    key = match[1].str();
                    value = match[2].str();
                } else {
                    // Fallback: treat the entire content as key with empty value
                    key = content;
                    value = "";
                }
            }
        }
        
        // Trim whitespace from key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (!key.empty()) {
            statusMap[key] = value;
        }
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

    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
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