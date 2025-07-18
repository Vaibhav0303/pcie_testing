# Build Instructions for PCIe Testing Framework

## Prerequisites

### Required Tools
Make sure you have the following installed on your system:

```bash
# On Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake libgtest-dev

# On CentOS/RHEL/Fedora
sudo yum install -y gcc-c++ cmake gtest-devel
# or for newer versions:
sudo dnf install -y gcc-c++ cmake gtest-devel

# On macOS (using Homebrew)
brew install cmake googletest
```

### Install and Build GoogleTest (if not available via package manager)

If GoogleTest is not available through your package manager:

```bash
# Clone and build GoogleTest
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..
rm -rf googletest
```

## Building the Project

### Step 1: Navigate to Project Directory
```bash
cd /path/to/your/pcie_testing
```

### Step 2: Create Build Directory
```bash
mkdir -p build
cd build
```

### Step 3: Configure with CMake
```bash
cmake ..
```

**Expected Output:**
```
-- Found GTest: /usr/local/lib/cmake/GTest/GTestConfig.cmake (found version "1.15.0")
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/pcie_testing/build
```

### Step 4: Build the Binary
```bash
make
```

**Expected Output:**
```
[ 25%] Building CXX object CMakeFiles/pcie_tests.dir/src/main.cpp.o
[ 50%] Building CXX object CMakeFiles/pcie_tests.dir/src/pcie_test.cpp.o
[ 75%] Building CXX object CMakeFiles/pcie_tests.dir/src/debugfs_handler.cpp.o
[100%] Linking CXX executable pcie_tests
[100%] Built target pcie_tests
```

### Step 5: Verify the Binary
```bash
ls -la pcie_tests
./pcie_tests --help
```

## Output Binary

After successful build, you'll find the final executable:
- **Location**: `build/pcie_tests`
- **Type**: Statically linked executable
- **Dependencies**: GoogleTest library

## Alternative Build Methods

### Quick Build (One Command)
```bash
mkdir -p build && cd build && cmake .. && make
```

### Debug Build
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build (Optimized)
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Clean Build
```bash
# From project root directory
rm -rf build
mkdir build && cd build
cmake .. && make
```

## Running the Tests

### Basic Execution
```bash
# From build directory
./pcie_tests
```

### With Google Test Options
```bash
# List all tests
./pcie_tests --gtest_list_tests

# Run specific test
./pcie_tests --gtest_filter="PCIeTest.AerEnableNode"

# Verbose output
./pcie_tests --gtest_print_time=1

# Generate XML report
./pcie_tests --gtest_output=xml:test_results.xml
```

## Requirements for Execution

### Hardware Requirements
- System with PCIe hardware
- Access to MSM PCIe debugfs interface at `/sys/kernel/debug/msm_pcie/`

### Permissions
```bash
# May need root privileges for debugfs access
sudo ./pcie_tests
```

### Runtime Files Generated
The test will create these files during execution:
- `baseline_status.txt` - Initial system status
- `current_status.txt` - Final system status

## Troubleshooting

### CMake Can't Find GTest
```bash
# Try specifying GTest path manually
cmake -DGTEST_ROOT=/usr/local ..
```

### Permission Denied on debugfs
```bash
# Check debugfs mount
mount | grep debugfs
# Run with sudo if needed
sudo ./pcie_tests
```

### Missing Libraries
```bash
# Check library dependencies
ldd pcie_tests
```

### Build Errors
```bash
# Clean and rebuild
make clean
make
```

## Project Structure

```
pcie_testing/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── BUILD_INSTRUCTIONS.md   # This file
├── .gitignore             # Git ignore rules
├── src/                   # Source code
│   ├── main.cpp           # Test runner
│   ├── pcie_test.cpp      # Test implementation
│   ├── pcie_test.h        # Test header
│   ├── debugfs_handler.cpp # Debugfs interface
│   └── debugfs_handler.h   # Debugfs header
└── build/                 # Build output (created)
    └── pcie_tests         # Final executable
```

## Final Binary Details

- **Name**: `pcie_tests`
- **Type**: Google Test executable
- **Purpose**: PCIe driver functionality testing
- **Platform**: Linux (requires MSM PCIe debugfs)
- **Size**: ~1-2 MB (depending on static linking)