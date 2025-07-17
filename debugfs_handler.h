#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <map>

class DebugfsHandler {
public:
    // Debugfs nodes
    static constexpr const char* DEBUGFS_PATH = "/sys/kernel/debug/msm_pcie/";
    static constexpr const char* NODE_AER_ENABLE = "aer_enable";
    static constexpr const char* NODE_BOOT_OPTION = "boot_option";
    static constexpr const char* NODE_CASE = "case";
    static constexpr const char* NODE_BASE_SEL = "base_sel";
    static constexpr const char* NODE_RC_SEL = "rc_sel";

    // Constructor
    DebugfsHandler();

    // Write to debugfs nodes
    void writeToNode(const std::string& node, int value);
    
    // Status handling
    void captureBaselineStatus();
    void saveStatus(const std::string& filename);
    std::map<std::string, std::string> compareWithBaseline();
    
    // Read from dmesg
    std::string getDmesgOutput(const std::string& filter = "");
    
    // Validate node values
    bool isValidNodeValue(const std::string& node, int value) const;

private:
    std::string getNodePath(const std::string& node) const;
    void clearDmesg();
    std::string baselineStatus;
    
    // Helper methods for status handling
    std::string getCurrentStatus();
    std::map<std::string, std::string> parseStatus(const std::string& status);
}; 