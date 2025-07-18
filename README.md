# PCIe Driver Tests

This project contains automated tests for PCIe controller driver debugfs nodes using the Google Test framework.

## Prerequisites

- CMake (version 3.10 or higher)
- Google Test framework
- Root access (for debugfs operations)
- Linux kernel with PCIe driver support

## Building the Tests

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure CMake:
```bash
cmake ..
```

3. Build the tests:
```bash
make
```

## Running the Tests

The tests need to be run with root privileges since they interact with debugfs nodes:

```bash
sudo ./pcie_tests
```

## Test Structure

The test suite includes:

1. Basic functionality tests for each debugfs node:
   - aer_enable (0/1)
   - boot_option (0/1)
   - case (0-9)
   - base_sel (0/1)
   - rc_sel (0/1)

2. Invalid input tests:
   - Testing invalid values for each node
   - Testing error handling

## Notes

- The tests assume that the PCIe driver is loaded and debugfs is mounted
- Tests will clear the dmesg buffer before each test case
- There is a small delay after writing to nodes to allow dmesg to update
- The tests filter dmesg output for "msm_pcie" to find relevant messages

## Troubleshooting

1. If tests fail with permission errors:
   - Ensure you're running with sudo
   - Check that debugfs is mounted at /sys/kernel/debug

2. If dmesg output is not found:
   - Check if the driver is loaded
   - Verify that the driver is logging to dmesg
   - Increase the sleep duration in verifyNodeOperation if needed 