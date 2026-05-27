#!/bin/bash

# Ensure script runs from the project root directory
if [ -f "run_all_tests.sh" ]; then
    cd ..
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test result counters
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# Log file
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
mkdir -p testcases/logs
LOG_FILE="testcases/logs/test_results_${TIMESTAMP}.log"

echo "========================================" | tee $LOG_FILE
echo "VEHICLE MONITORING SYSTEM - TEST SUITE" | tee -a $LOG_FILE
echo "========================================" | tee -a $LOG_FILE
echo "Started: $(date)" | tee -a $LOG_FILE
echo "" | tee -a $LOG_FILE

# Function to print test header
print_test_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# Function to print test result
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ PASS${NC}: $2" | tee -a $LOG_FILE
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAIL${NC}: $2" | tee -a $LOG_FILE
        ((TESTS_FAILED++))
    fi
    echo "" | tee -a $LOG_FILE
}

# Function to skip test
skip_test() {
    echo -e "${YELLOW}⏭️  SKIP${NC}: $1" | tee -a $LOG_FILE
    ((TESTS_SKIPPED++))
    echo "" | tee -a $LOG_FILE
}

#==============================================================================
# PRE-CHECKS
#==============================================================================

print_test_header "PRE-REQUISITE CHECKS"

# Check if project is built
echo "Checking if project is built..." | tee -a $LOG_FILE
if [ ! -f "build/VehicleMonitoringSystem" ]; then
    echo -e "${YELLOW}⚠️  Executable not found. Building project...${NC}" | tee -a $LOG_FILE
    ./build.sh
    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ Build failed! Cannot proceed with tests.${NC}" | tee -a $LOG_FILE
        exit 1
    fi
fi
print_result 0 "Build check"

# Check if logs directory exists
echo "Checking logs directory..." | tee -a $LOG_FILE
if [ ! -d "logs" ]; then
    mkdir -p logs
fi
print_result 0 "Logs directory"

# Check if config file exists
echo "Checking config file..." | tee -a $LOG_FILE
if [ ! -f "data/config.json" ]; then
    echo -e "${RED}❌ config.json not found!${NC}" | tee -a $LOG_FILE
    exit 1
fi
print_result 0 "Configuration file"

#==============================================================================
# SECTION 1: BUILD TESTS
#==============================================================================

print_test_header "SECTION 1: BUILD TESTS"

echo "TC-BUILD-001: Clean compilation" | tee -a $LOG_FILE
cd build
make clean > /dev/null 2>&1
make -j$(nproc) > /dev/null 2>&1
BUILD_RESULT=$?
cd ..
print_result $BUILD_RESULT "Clean compilation"

echo "TC-BUILD-002: Check for compiler warnings" | tee -a $LOG_FILE
cd build
WARNING_COUNT=$(make 2>&1 | grep -i "warning" | wc -l)
cd ..
if [ $WARNING_COUNT -eq 0 ]; then
    print_result 0 "No compiler warnings"
else
    echo "  Found $WARNING_COUNT warnings" | tee -a $LOG_FILE
    print_result 1 "Compiler warnings found"
fi

#==============================================================================
# SECTION 2: SMOKE TEST
#==============================================================================

print_test_header "SECTION 2: SMOKE TEST"

echo "TC-SMOKE-001: Application starts without crash" | tee -a $LOG_FILE
# Start application in background and kill after 5 seconds
timeout 5s ./build/VehicleMonitoringSystem > /dev/null 2>&1
EXIT_CODE=$?
if [ $EXIT_CODE -eq 124 ] || [ $EXIT_CODE -eq 0 ]; then
    # 124 = timeout (expected), 0 = normal exit
    print_result 0 "Application started successfully"
else
    print_result 1 "Application crashed on startup"
fi

#==============================================================================
# SECTION 3: UNIT TESTS (If compiled with -DBUILD_TESTS=ON)
#==============================================================================

print_test_header "SECTION 3: UNIT TESTS"

if [ -d "build/tests" ]; then
    echo "Running unit tests..." | tee -a $LOG_FILE
    cd build
    ctest --output-on-failure > ../testcases/unit_test_output.txt 2>&1
    UNIT_TEST_RESULT=$?
    cd ..
    print_result $UNIT_TEST_RESULT "Unit tests"
else
    skip_test "Unit tests not compiled (use -DBUILD_TESTS=ON)"
fi

#==============================================================================
# SECTION 4: LOG FILE TESTS
#==============================================================================

print_test_header "SECTION 4: LOG FILE TESTS"

# Clean old logs
rm -f logs/vehicle_log.txt

echo "TC-LOG-001: Log file creation" | tee -a $LOG_FILE
timeout 3s ./build/VehicleMonitoringSystem > /dev/null 2>&1
if [ -f "logs/vehicle_log.txt" ]; then
    print_result 0 "Log file created"
else
    print_result 1 "Log file not created"
fi

echo "TC-LOG-002: Log entries have timestamps" | tee -a $LOG_FILE
if [ -f "logs/vehicle_log.txt" ]; then
    TIMESTAMP_COUNT=$(grep -E "\[[0-9]{4}-[0-9]{2}-[0-9]{2}" logs/vehicle_log.txt | wc -l)
    if [ $TIMESTAMP_COUNT -gt 0 ]; then
        print_result 0 "Log entries have timestamps"
    else
        print_result 1 "Log entries missing timestamps"
    fi
else
    skip_test "Log file not available"
fi

#==============================================================================
# SECTION 5: CONFIGURATION TESTS
#==============================================================================

print_test_header "SECTION 5: CONFIGURATION TESTS"

echo "TC-CONFIG-001: Valid JSON parsing" | tee -a $LOG_FILE
# Application should start if config is valid
timeout 2s ./build/VehicleMonitoringSystem > /dev/null 2>&1
print_result $? "JSON config loaded"

echo "TC-CONFIG-002: Profile files exist" | tee -a $LOG_FILE
PROFILE_COUNT=0
[ -f "data/driver_profiles/eco_mode.json" ] && ((PROFILE_COUNT++))
[ -f "data/driver_profiles/sport_mode.json" ] && ((PROFILE_COUNT++))
[ -f "data/driver_profiles/comfort_mode.json" ] && ((PROFILE_COUNT++))

if [ $PROFILE_COUNT -eq 3 ]; then
    print_result 0 "All 3 driver profiles present"
else
    print_result 1 "Missing driver profiles ($PROFILE_COUNT/3 found)"
fi

#==============================================================================
# SECTION 6: MEMORY LEAK TEST
#==============================================================================

print_test_header "SECTION 6: MEMORY LEAK TEST"

if command -v valgrind &> /dev/null; then
    echo "TC-MEM-001: Memory leak detection" | tee -a $LOG_FILE
    echo "  Running valgrind (this may take 30 seconds)..." | tee -a $LOG_FILE
    
    timeout 10s valgrind --leak-check=full \
                         --error-exitcode=1 \
                         ./build/VehicleMonitoringSystem \
                         > testcases/valgrind_output.txt 2>&1
    
    VALGRIND_RESULT=$?
    LEAKED_BYTES=$(grep "definitely lost:" testcases/valgrind_output.txt | awk '{print $4}' | sed 's/,//g')
    
    if [ "$LEAKED_BYTES" == "0" ]; then
        print_result 0 "No memory leaks detected"
    else
        echo "  Leaked: $LEAKED_BYTES bytes" | tee -a $LOG_FILE
        print_result 1 "Memory leaks detected"
    fi
else
    skip_test "Valgrind not installed"
fi

#==============================================================================
# SECTION 7: THREAD TESTS
#==============================================================================

print_test_header "SECTION 7: THREADING TESTS"

echo "TC-THREAD-001: Multiple threads running" | tee -a $LOG_FILE
# Start app in background
./build/VehicleMonitoringSystem > /dev/null 2>&1 &
APP_PID=$!
sleep 2

# Count threads
THREAD_COUNT=$(ps -T -p $APP_PID | wc -l)
# Subtract header line
THREAD_COUNT=$((THREAD_COUNT - 1))

kill $APP_PID 2>/dev/null
wait $APP_PID 2>/dev/null

if [ $THREAD_COUNT -ge 4 ]; then
    echo "  Found $THREAD_COUNT threads" | tee -a $LOG_FILE
    print_result 0 "Multiple threads detected"
else
    echo "  Found only $THREAD_COUNT threads" | tee -a $LOG_FILE
    print_result 1 "Expected 4+ threads"
fi

#==============================================================================
# SUMMARY
#==============================================================================

echo "" | tee -a $LOG_FILE
echo "========================================" | tee -a $LOG_FILE
echo "TEST SUMMARY" | tee -a $LOG_FILE
echo "========================================" | tee -a $LOG_FILE
echo -e "${GREEN}Passed:${NC}  $TESTS_PASSED" | tee -a $LOG_FILE
echo -e "${RED}Failed:${NC}  $TESTS_FAILED" | tee -a $LOG_FILE
echo -e "${YELLOW}Skipped:${NC} $TESTS_SKIPPED" | tee -a $LOG_FILE
echo "----------------------------------------" | tee -a $LOG_FILE

TOTAL_TESTS=$((TESTS_PASSED + TESTS_FAILED))
if [ $TOTAL_TESTS -gt 0 ]; then
    PASS_RATE=$((TESTS_PASSED * 100 / TOTAL_TESTS))
    echo "Pass Rate: $PASS_RATE%" | tee -a $LOG_FILE
fi

echo "========================================" | tee -a $LOG_FILE
echo "Completed: $(date)" | tee -a $LOG_FILE
echo "Log file: $LOG_FILE" | tee -a $LOG_FILE
echo "========================================" | tee -a $LOG_FILE

# Exit with failure if any tests failed
if [ $TESTS_FAILED -gt 0 ]; then
    exit 1
else
    exit 0
fi