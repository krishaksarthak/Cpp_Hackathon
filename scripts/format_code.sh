#!/bin/bash

echo "========================================"
echo "Formatting Code with clang-format"
echo "========================================"

if ! command -v clang-format &> /dev/null; then
    echo "❌ clang-format not installed"
    echo "Install with: sudo apt-get install clang-format"
    exit 1
fi

find include/ src/ -iname "*.hpp" -o -iname "*.cpp" | xargs clang-format -i -style=Google

echo "✅ Code formatting complete!"