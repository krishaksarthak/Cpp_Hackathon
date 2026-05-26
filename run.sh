#!/bin/bash

echo "========================================"
echo "Building and Running Vehicle Monitoring System"
echo "Compiler: GCC"
echo "Build System: CMake"
echo "========================================"

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake is not installed or not in PATH."
    exit 1
fi

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "❌ make is not installed or not in PATH."
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Create logs directory if it doesn't exist
mkdir -p ../logs

echo ""
echo "[1/3] Configuring CMake..."
echo "[INFO] Saving detailed output to logs/build.log"
cmake .. > ../logs/build.log 2>&1
if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed. Check logs/build.log"
    cd ..
    exit 1
fi

echo ""
echo "[2/3] Building Project..."
make -j4 >> ../logs/build.log 2>&1
if [ $? -ne 0 ]; then
    echo "❌ Build failed. Check logs/build.log"
    cd ..
    exit 1
fi

echo ""
echo "[3/3] Running Tests..."
ctest -V >> ../logs/build.log 2>&1

echo ""
echo "========================================"
echo "Launching Application"
echo "========================================"
echo ""

# Return to root directory so that data/ and logs/ folders resolve correctly
cd ..

# Run the application
build/VehicleMonitoringSystem

echo ""
echo "========================================"
echo "Application terminated"
echo "Check logs/ directory for event logs"
echo "========================================"