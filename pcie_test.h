#pragma once

#include <gtest/gtest.h>
#include "debugfs_handler.h"
#include <map>
#include <string>

class PCIeTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    DebugfsHandler debugfs;
    
    // Helper methods
    void verifyNodeEffect(const std::string& node, int value, const std::string& expectedKey);
    void saveStatusToFile(const std::string& filename);
    
    // Status file paths
    static constexpr const char* BASELINE_STATUS_FILE = "baseline_status.txt";
    static constexpr const char* CURRENT_STATUS_FILE = "current_status.txt";
}; 