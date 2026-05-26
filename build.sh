#!/bin/bash

echo "========================================"
echo "Building Vehicle Monitoring System"
echo "========================================"

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
echo "Compiling..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "✅ Build successful!"
    echo "========================================"
    echo "Run with: ./run.sh"
    echo ""
else
    echo ""
    echo "========================================"
    echo "❌ Build failed!"
    echo "========================================"
    exit 1
fi