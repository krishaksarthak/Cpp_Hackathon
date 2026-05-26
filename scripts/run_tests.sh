#!/bin/bash

echo "========================================"
echo "Running Unit Tests"
echo "========================================"

if [ ! -d "build" ]; then
    echo "Building project first..."
    ./build.sh
fi

cd build
ctest --output-on-failure

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ All tests passed!"
else
    echo ""
    echo "❌ Some tests failed"
    exit 1
fi