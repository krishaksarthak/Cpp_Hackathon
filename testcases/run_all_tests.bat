@echo off
setlocal enabledelayedexpansion

:: Test result counters
set TESTS_PASSED=0
set TESTS_FAILED=0
set TESTS_SKIPPED=0

:: Log file
set TIMESTAMP=%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
set LOG_FILE=testcases\test_results_%TIMESTAMP%.log

echo ======================================== > %LOG_FILE%
echo VEHICLE MONITORING SYSTEM - TEST SUITE >> %LOG_FILE%
echo ======================================== >> %LOG_FILE%
echo Started: %date% %time% >> %LOG_FILE%
echo. >> %LOG_FILE%

echo ========================================
echo VEHICLE MONITORING SYSTEM - TEST SUITE
echo ========================================
echo.

::==============================================================================
:: PRE-CHECKS
::==============================================================================

echo ========================================
echo PRE-REQUISITE CHECKS
echo ========================================

:: Check if project is built
echo Checking if project is built...
if not exist "build\VehicleMonitoringSystem.exe" (
    echo [WARNING] Executable not found. Building project...
    call build.bat
    if errorlevel 1 (
        echo [ERROR] Build failed! Cannot proceed with tests.
        exit /b 1
    )
)
echo [PASS] Build check
set /a TESTS_PASSED+=1

:: Check logs directory
echo Checking logs directory...
if not exist "logs" mkdir logs
echo [PASS] Logs directory
set /a TESTS_PASSED+=1

:: Check config file
echo Checking config file...
if not exist "data\config.json" (
    echo [FAIL] config.json not found!
    set /a TESTS_FAILED+=1
    exit /b 1
)
echo [PASS] Configuration file
set /a TESTS_PASSED+=1

::==============================================================================
:: SECTION 1: BUILD TESTS
::==============================================================================

echo.
echo ========================================
echo SECTION 1: BUILD TESTS
echo ========================================

echo TC-BUILD-001: Clean compilation
cd build
cmake --build . --clean-first > nul 2>&1
if errorlevel 1 (
    echo [FAIL] Clean compilation
    set /a TESTS_FAILED+=1
) else (
    echo [PASS] Clean compilation
    set /a TESTS_PASSED+=1
)
cd ..

::==============================================================================
:: SECTION 2: SMOKE TEST
::==============================================================================

echo.
echo ========================================
echo SECTION 2: SMOKE TEST
echo ========================================

echo TC-SMOKE-001: Application starts without crash
start /B build\VehicleMonitoringSystem.exe > nul 2>&1
timeout /t 3 /nobreak > nul
taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1

if exist "logs\vehicle_events.log" (
    echo [PASS] Application started successfully
    set /a TESTS_PASSED+=1
) else (
    echo [FAIL] Application did not start properly
    set /a TESTS_FAILED+=1
)

::==============================================================================
:: SECTION 3: LOG FILE TESTS
::==============================================================================

echo.
echo ========================================
echo SECTION 3: LOG FILE TESTS
echo ========================================

:: Clean old logs
if exist "logs\vehicle_events.log" del /Q logs\vehicle_events.log

echo TC-LOG-001: Log file creation
start /B build\VehicleMonitoringSystem.exe > nul 2>&1
timeout /t 3 /nobreak > nul
taskkill /F /IM VehicleMonitoringSystem.exe > nul 2>&1

if exist "logs\vehicle_events.log" (
    echo [PASS] Log file created
    set /a TESTS_PASSED+=1
) else (
    echo [FAIL] Log file not created
    set /a TESTS_FAILED+=1
)

echo TC-LOG-002: Log entries have timestamps
if exist "logs\vehicle_events.log" (
    findstr /R "\[[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]" logs\vehicle_events.log > nul
    if errorlevel 1 (
        echo [FAIL] Log entries missing timestamps
        set /a TESTS_FAILED+=1
    ) else (
        echo [PASS] Log entries have timestamps
        set /a TESTS_PASSED+=1
    )
) else (
    echo [SKIP] Log file not available
    set /a TESTS_SKIPPED+=1
)

::==============================================================================
:: SECTION 4: CONFIGURATION TESTS
::==============================================================================

echo.
echo ========================================
echo SECTION 4: CONFIGURATION TESTS
echo ========================================

echo TC-CONFIG-001: Profile files exist
set PROFILE_COUNT=0
if exist "data\driver_profiles\eco_mode.json" set /a PROFILE_COUNT+=1
if exist "data\driver_profiles\sport_mode.json" set /a PROFILE_COUNT+=1
if exist "data\driver_profiles\comfort_mode.json" set /a PROFILE_COUNT+=1

if %PROFILE_COUNT%==3 (
    echo [PASS] All 3 driver profiles present
    set /a TESTS_PASSED+=1
) else (
    echo [FAIL] Missing driver profiles (%PROFILE_COUNT%/3 found^)
    set /a TESTS_FAILED+=1
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

set /a TOTAL_TESTS=%TESTS_PASSED%+%TESTS_FAILED%
if %TOTAL_TESTS% GTR 0 (
    set /a PASS_RATE=(%TESTS_PASSED%*100)/%TOTAL_TESTS%
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