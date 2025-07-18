# PCIe Driver Tests Makefile
# Project: PCIe_Driver_Tests
# Description: Makefile to build PCIe driver tests without CMake

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2
INCLUDES = -I./src -I/usr/include/gtest
LIBS = -lgtest -lgtest_main -lpthread

# Directories
SRCDIR = src
BUILDDIR = build
TARGET = pcie_tests

# Source files
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/pcie_test.cpp \
          $(SRCDIR)/debugfs_handler.cpp

# Object files (place them in build directory)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Default target
.PHONY: all clean install uninstall help

all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build the main target
$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)
	@echo "Clean complete"

# Install target (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installed $(TARGET) to /usr/local/bin/"

# Uninstall target
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET) from /usr/local/bin/"

# Run tests (requires root privileges)
test: $(TARGET)
	@echo "Running tests (requires root privileges)..."
	sudo ./$(TARGET)

# Check if dependencies are installed
check-deps:
	@echo "Checking dependencies..."
	@which $(CXX) > /dev/null || (echo "Error: $(CXX) not found" && exit 1)
	@pkg-config --exists gtest || (echo "Error: Google Test not found. Install with: sudo apt-get install libgtest-dev" && exit 1)
	@echo "All dependencies found"

# Debug build with additional flags
debug: CXXFLAGS += -g -DDEBUG -O0
debug: $(TARGET)
	@echo "Debug build complete: $(TARGET)"

# Release build with optimizations
release: CXXFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)
	@echo "Release build complete: $(TARGET)"

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the project (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install binary to /usr/local/bin"
	@echo "  uninstall  - Remove binary from /usr/local/bin"
	@echo "  test       - Build and run tests (requires sudo)"
	@echo "  check-deps - Check if required dependencies are installed"
	@echo "  debug      - Build with debug flags"
	@echo "  release    - Build optimized release version"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make           # Build the project"
	@echo "  make clean     # Clean build files"
	@echo "  make test      # Build and run tests"
	@echo "  make debug     # Build debug version"
	@echo "  make install   # Install to system"

# Dependencies for object files (auto-generated)
$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp
$(BUILDDIR)/pcie_test.o: $(SRCDIR)/pcie_test.cpp $(SRCDIR)/pcie_test.h $(SRCDIR)/debugfs_handler.h
$(BUILDDIR)/debugfs_handler.o: $(SRCDIR)/debugfs_handler.cpp $(SRCDIR)/debugfs_handler.h