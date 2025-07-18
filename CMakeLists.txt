cmake_minimum_required(VERSION 3.10)
project(PCIe_Driver_Tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add source files
add_executable(pcie_tests
    src/main.cpp
    src/pcie_test.cpp
    src/debugfs_handler.cpp
)

# Link against GTest
target_link_libraries(pcie_tests
    ${GTEST_LIBRARIES}
    pthread
) 