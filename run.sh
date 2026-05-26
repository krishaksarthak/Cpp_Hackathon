#!/bin/bash

echo "========================================"
echo "Running Vehicle Monitoring System"
echo "========================================"

# Check if build exists
if [ ! -f "build/VehicleMonitoringSystem" ]; then
    echo "❌ Executable not found. Please run ./build.sh first"
    exit 1
fi

# Create logs directory if doesn't exist
mkdir -p logs

# Run the application
cd build
./VehicleMonitoringSystem

echo ""
echo "========================================"
echo "Application terminated"
echo "Check logs/ directory for event logs"
echo "========================================"