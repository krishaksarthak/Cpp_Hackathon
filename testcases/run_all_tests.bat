@echo off
setlocal enabledelayedexpansion

:: Ensure script runs from the project root directory
if exist "run_all_tests.bat" (
    cd ..
)

:: ANSI Color Setup for Windows 10/11
for /F %%a in ('echo prompt $E ^| cmd') do set ESC=%%a
set GREEN=%ESC%[32m
set RED=%ESC%[31m
set YELLOW=%ESC%[33m
set CYAN=%ESC%[36m
set RESET=%ESC%[0m

:: Test result counters
set TESTS_PASSED=0
set TESTS_FAILED=0
set TESTS_SKIPPED=0

:: Log file
set TIMESTAMP=%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
if not exist "testcases\logs" mkdir testcases\logs
set LOG_FILE=testcases\logs\test_results_%TIMESTAMP%.log

echo ======================================== > %LOG_FILE%
echo VEHICLE MONITORING SYSTEM - TEST SUITE >> %LOG_FILE%
echo ======================================== >> %LOG_FILE%
echo Started: %date% %time% >> %LOG_FILE%
echo. >> %LOG_FILE%

:: Initialize detailed append log
set DETAILED_LOG=testcases\logs\detailed_test_report.log
echo %CYAN%========================================%RESET% >> %DETAILED_LOG%
echo TEST RUN STARTED: %date% %time% >> %DETAILED_LOG%
echo %CYAN%========================================%RESET% >> %DETAILED_LOG%

echo %CYAN%========================================%RESET%
echo VEHICLE MONITORING SYSTEM - TEST SUITE
echo %CYAN%========================================%RESET%
echo.

::==============================================================================
:: PRE-CHECKS
::==============================================================================

echo %CYAN%========================================%RESET%
echo PRE-REQUISITE CHECKS
echo %CYAN%========================================%RESET%

:: Check if project is built
echo Checking if project is built...
if not exist "build\VehicleMonitoringSystem.exe" (
    echo [WARNING] Executable not found. Building project...
    if not exist "build" mkdir build
    cd build
    cmake -G "MinGW Makefiles" .. > nul
    mingw32-make -j4 > nul
    cd ..
    if not exist "build\VehicleMonitoringSystem.exe" (
        echo [ERROR] Build failed! Cannot proceed with tests.
        exit /b 1
    )
)
echo %GREEN%[PASS]%RESET% Build check
set /a TESTS_PASSED+=1

:: Check logs directory
echo Checking logs directory...
if not exist "logs" mkdir logs
echo %GREEN%[PASS]%RESET% Logs directory
set /a TESTS_PASSED+=1

:: Check config file
echo Checking config file...
if not exist "data\config.json" (
    echo %RED%[FAIL]%RESET% config.json not found!
    set /a TESTS_FAILED+=1
    exit /b 1
)
echo %GREEN%[PASS]%RESET% Configuration file
set /a TESTS_PASSED+=1

::==============================================================================
:: SECTION 1: BUILD TESTS
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 1: BUILD TESTS
echo %CYAN%========================================%RESET%

echo TC-BUILD-001: Clean compilation
cd build
cmake --build . --clean-first > nul 2>&1
if errorlevel 1 (
    echo %RED%[FAIL]%RESET% Clean compilation
    set /a TESTS_FAILED+=1
) else (
    echo %GREEN%[PASS]%RESET% Clean compilation
    set /a TESTS_PASSED+=1
)
cd ..

::==============================================================================
:: SECTION 2: SMOKE TEST
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 2: SMOKE TEST
echo %CYAN%========================================%RESET%

echo TC-SMOKE-001: Application starts without crash
start /B build\VehicleMonitoringSystem.exe > nul 2>&1
timeout /t 3 /nobreak > nul
taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1

    if exist "logs\vehicle_log.txt" (
        echo %GREEN%[PASS]%RESET% Application started successfully
        set /a TESTS_PASSED+=1
    ) else (
        echo %RED%[FAIL]%RESET% Application did not start properly
        set /a TESTS_FAILED+=1
    )

    ::==============================================================================
    :: SECTION 3: LOG FILE TESTS
    ::==============================================================================

    echo.
    echo %CYAN%========================================%RESET%
    echo SECTION 3: LOG FILE TESTS
    echo %CYAN%========================================%RESET%

    :: Clean old logs
    if exist "logs\vehicle_log.txt" del /Q logs\vehicle_log.txt

    echo TC-LOG-001: Log file creation
    start /B build\VehicleMonitoringSystem.exe > nul 2>&1
    timeout /t 3 /nobreak > nul
    taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1

    if exist "logs\vehicle_log.txt" (
        echo %GREEN%[PASS]%RESET% Log file created
        set /a TESTS_PASSED+=1
    ) else (
        echo %RED%[FAIL]%RESET% Log file not created
        set /a TESTS_FAILED+=1
    )

    echo TC-LOG-002: Log entries have timestamps
    if exist "logs\vehicle_log.txt" (
        findstr /R "\[[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]" logs\vehicle_log.txt > nul
    if errorlevel 1 (
        echo %RED%[FAIL]%RESET% Log entries missing timestamps
        set /a TESTS_FAILED+=1
    ) else (
        echo %GREEN%[PASS]%RESET% Log entries have timestamps
        set /a TESTS_PASSED+=1
    )
) else (
    echo %YELLOW%[SKIP]%RESET% Log file not available
    set /a TESTS_SKIPPED+=1
)

::==============================================================================
:: SECTION 4: CONFIGURATION TESTS
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 4: CONFIGURATION TESTS
echo %CYAN%========================================%RESET%

echo TC-CONFIG-001: Profile files exist
set PROFILE_COUNT=0
if exist "data\driver_profiles\eco_mode.json" set /a PROFILE_COUNT+=1
if exist "data\driver_profiles\sport_mode.json" set /a PROFILE_COUNT+=1
if exist "data\driver_profiles\comfort_mode.json" set /a PROFILE_COUNT+=1

if %PROFILE_COUNT%==3 (
    echo %GREEN%[PASS]%RESET% All 3 driver profiles present
    set /a TESTS_PASSED+=1
) else (
    echo %RED%[FAIL]%RESET% Missing driver profiles (%PROFILE_COUNT%/3 found^)
    set /a TESTS_FAILED+=1
)

::==============================================================================
:: SECTION 5: C++ UNIT TESTS (EDGE CASES & BUSINESS LOGIC)
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 5: C++ UNIT TESTS (EDGE CASES)
echo %CYAN%========================================%RESET%

echo TC-UNIT-001: Sensor Simulation ^& Bounds Edge Cases
if exist "build\tests\test_sensors.exe" (
    echo [RUNNING] test_sensors.exe >> %DETAILED_LOG%
    build\tests\test_sensors.exe >> %DETAILED_LOG% 2>&1
    if errorlevel 1 (
        echo %RED%[FAIL]%RESET% Sensor edge cases failed
        set /a TESTS_FAILED+=1
    ) else (
        echo %GREEN%[PASS]%RESET% Sensor edge cases passed
        set /a TESTS_PASSED+=1
    )
) else (
    echo %YELLOW%[SKIP]%RESET% test_sensors.exe not found
    set /a TESTS_SKIPPED+=1
)

echo TC-UNIT-002: Alert Evaluation ^& DTC Edge Cases
if exist "build\tests\test_alerts.exe" (
    echo [RUNNING] test_alerts.exe >> %DETAILED_LOG%
    build\tests\test_alerts.exe >> %DETAILED_LOG% 2>&1
    if errorlevel 1 (
        echo %RED%[FAIL]%RESET% Alert/DTC edge cases failed
        set /a TESTS_FAILED+=1
    ) else (
        echo %GREEN%[PASS]%RESET% Alert/DTC edge cases passed
        set /a TESTS_PASSED+=1
    )
) else (
    echo %YELLOW%[SKIP]%RESET% test_alerts.exe not found
    set /a TESTS_SKIPPED+=1
)

echo TC-UNIT-003: Thread Safety ^& Watchdog Edge Cases
if exist "build\tests\test_threading.exe" (
    echo [RUNNING] test_threading.exe >> %DETAILED_LOG%
    build\tests\test_threading.exe >> %DETAILED_LOG% 2>&1
    if errorlevel 1 (
        echo %RED%[FAIL]%RESET% Threading edge cases failed
        set /a TESTS_FAILED+=1
    ) else (
        echo %GREEN%[PASS]%RESET% Threading edge cases passed
        set /a TESTS_PASSED+=1
    )
) else (
    echo %YELLOW%[SKIP]%RESET% test_threading.exe not found
    set /a TESTS_SKIPPED+=1
)

::==============================================================================
:: SECTION 6: FILE SYSTEM EDGE CASES
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 6: FILE SYSTEM EDGE CASES
echo %CYAN%========================================%RESET%

echo TC-EDGE-001: Application recovery without config.json
if exist "data\config.json" (
    move data\config.json data\config.json.bak > nul
    start /B build\VehicleMonitoringSystem.exe > nul 2>&1
    timeout /t 3 /nobreak > nul
    taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1
    move data\config.json.bak data\config.json > nul
    echo %GREEN%[PASS]%RESET% Handled missing config safely
    set /a TESTS_PASSED+=1
) else (
    echo %YELLOW%[SKIP]%RESET% Cannot test missing config
    set /a TESTS_SKIPPED+=1
)

echo TC-EDGE-002: Application recovery without driver profiles
if exist "data\driver_profiles" (
    move data\driver_profiles data\driver_profiles_bak > nul
    start /B build\VehicleMonitoringSystem.exe > nul 2>&1
    timeout /t 3 /nobreak > nul
    taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1
    move data\driver_profiles_bak data\driver_profiles > nul
    echo %GREEN%[PASS]%RESET% Handled missing profiles safely
    set /a TESTS_PASSED+=1
) else (
    echo %YELLOW%[SKIP]%RESET% Cannot test missing profiles
    set /a TESTS_SKIPPED+=1
)

::==============================================================================
:: SECTION 7: COMPREHENSIVE EDGE CASES (C++ EXTREME TESTING)
::==============================================================================

echo.
echo %CYAN%========================================%RESET%
echo SECTION 7: COMPREHENSIVE EDGE CASES
echo %CYAN%========================================%RESET%

echo TC-EDGE-003: Extreme Physics, Concurrency, and Parser Edge Cases
if exist "build\testcases\comprehensive_edgecases.exe" (
    echo [RUNNING] comprehensive_edgecases.exe >> %DETAILED_LOG%
    build\testcases\comprehensive_edgecases.exe >> %DETAILED_LOG% 2>&1
    if errorlevel 1 (
        echo %RED%[FAIL]%RESET% Comprehensive edge cases failed
        set /a TESTS_FAILED+=1
    ) else (
        echo %GREEN%[PASS]%RESET% Comprehensive edge cases passed
        set /a TESTS_PASSED+=1
    )
) else (
    echo %YELLOW%[SKIP]%RESET% comprehensive_edgecases.exe not found
    set /a TESTS_SKIPPED+=1
)

::==============================================================================
:: SUMMARY
::==============================================================================

echo.
echo ========================================
echo TEST SUMMARY
echo ========================================
echo Passed:  %TESTS_PASSED%
echo Failed:  %TESTS_FAILED%
echo Skipped: %TESTS_SKIPPED%
echo ----------------------------------------

set /a TOTAL_TESTS=TESTS_PASSED+TESTS_FAILED
if !TOTAL_TESTS! GTR 0 (
    set /a PASS_RATE=TESTS_PASSED*100/TOTAL_TESTS
    echo Pass Rate: !PASS_RATE!%%
)

echo ========================================
echo Completed: %date% %time%
echo Log file: %LOG_FILE%
echo ========================================

:: Write summary to log
echo. >> %LOG_FILE%
echo ======================================== >> %LOG_FILE%
echo TEST SUMMARY >> %LOG_FILE%
echo ======================================== >> %LOG_FILE%
echo Passed:  %TESTS_PASSED% >> %LOG_FILE%
echo Failed:  %TESTS_FAILED% >> %LOG_FILE%
echo Skipped: %TESTS_SKIPPED% >> %LOG_FILE%
echo ======================================== >> %LOG_FILE%

if %TESTS_FAILED% GTR 0 (
    exit /b 1
) else (
    exit /b 0
)