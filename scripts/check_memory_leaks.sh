#!/bin/bash

echo "========================================"
echo "Checking for Memory Leaks"
echo "========================================"

if ! command -v valgrind &> /dev/null; then
    echo "❌ Valgrind not installed"
    echo "Install with: sudo apt-get install valgrind"
    exit 1
fi

if [ ! -f "build/VehicleMonitoringSystem" ]; then
    echo "❌ Executable not found. Run ./build.sh first"
    exit 1
fi

cd build
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=../valgrind-report.txt \
         ./VehicleMonitoringSystem

echo ""
echo "========================================"
echo "Memory check complete!"
echo "Report saved to: valgrind-report.txt"
echo "========================================"