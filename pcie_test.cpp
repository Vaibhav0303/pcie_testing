#include "pcie_test.h"
#include <thread>
#include <chrono>

void PCIeTest::SetUp() {
    // Get initial status
    debugfs.captureBaselineStatus();
    saveStatusToFile(BASELINE_STATUS_FILE);
}

void PCIeTest::TearDown() {
    // Save final status for debugging purposes
    saveStatusToFile(CURRENT_STATUS_FILE);
}

void PCIeTest::verifyNodeEffect(const std::string& node, int value, const std::string& expectedKey) {
    // Write to the node
    debugfs.writeToNode(node, value);
    
    // Compare with baseline
    auto differences = debugfs.compareWithBaseline();
    
    // Check if the expected key exists in the differences
    ASSERT_TRUE(differences.find(expectedKey) != differences.end())
        << "Expected change in " << expectedKey << " not found in status differences";
        
    // Save current status for debugging
    saveStatusToFile(CURRENT_STATUS_FILE);
}

void PCIeTest::saveStatusToFile(const std::string& filename) {
    debugfs.saveStatus(filename);
}

// Test cases for aer_enable node
TEST_F(PCIeTest, AerEnableNode) {
    verifyNodeEffect(DebugfsHandler::NODE_AER_ENABLE, 0, "AER Status");
    verifyNodeEffect(DebugfsHandler::NODE_AER_ENABLE, 1, "AER Status");
}

// Test cases for boot_option node
TEST_F(PCIeTest, BootOptionNode) {
    verifyNodeEffect(DebugfsHandler::NODE_BOOT_OPTION, 0, "Boot Option");
    verifyNodeEffect(DebugfsHandler::NODE_BOOT_OPTION, 1, "Boot Option");
}

// Test cases for base_sel node
TEST_F(PCIeTest, BaseSelNode) {
    verifyNodeEffect(DebugfsHandler::NODE_BASE_SEL, 0, "Base Selection");
    verifyNodeEffect(DebugfsHandler::NODE_BASE_SEL, 1, "Base Selection");
}

// Test cases for rc_sel node
TEST_F(PCIeTest, RcSelNode) {
    verifyNodeEffect(DebugfsHandler::NODE_RC_SEL, 0, "RC Selection");
    verifyNodeEffect(DebugfsHandler::NODE_RC_SEL, 1, "RC Selection");
}

// Test cases for case node
TEST_F(PCIeTest, CaseNode) {
    // Example test for L0s support using case node
    verifyNodeEffect(DebugfsHandler::NODE_CASE, 1, "L0s Support"); // Disable L0s
    verifyNodeEffect(DebugfsHandler::NODE_CASE, 2, "L0s Support"); // Enable L0s
    
    // Add more test cases for other case node options
    // Example (you'll need to replace these with actual options from your driver):
    // verifyNodeEffect(DebugfsHandler::NODE_CASE, 3, "L1 Support");
    // verifyNodeEffect(DebugfsHandler::NODE_CASE, 4, "ASPM Support");
    // etc...
}

// Test invalid values
TEST_F(PCIeTest, InvalidValues) {
    // Test invalid value for case node
    EXPECT_THROW(debugfs.writeToNode(DebugfsHandler::NODE_CASE, 10), std::invalid_argument);
    
    // Test invalid values for binary nodes
    EXPECT_THROW(debugfs.writeToNode(DebugfsHandler::NODE_AER_ENABLE, 2), std::invalid_argument);
    EXPECT_THROW(debugfs.writeToNode(DebugfsHandler::NODE_BOOT_OPTION, -1), std::invalid_argument);
} 