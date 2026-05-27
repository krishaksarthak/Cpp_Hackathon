@echo off
setlocal enabledelayedexpansion

:: Create logs directory
if not exist "logs" mkdir logs

:: Generate Timestamp
set TIMESTAMP=%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
set LOG_FILE=logs\edgecase_report_%TIMESTAMP%.log

echo ========================================
echo COMPILING EDGE CASES
echo ========================================
cd ..
if not exist "build" mkdir build
cd build
cmake -G "MinGW Makefiles" .. > nul
mingw32-make comprehensive_edgecases -j4 > nul

if errorlevel 1 (
    echo [ERROR] Failed to compile comprehensive_edgecases.cpp
    cd ..\testcases
    exit /b 1
)

echo ========================================
echo RUNNING EDGE CASES
echo ========================================
echo Saving report to %LOG_FILE%

:: Run the executable and pipe to log file, but also display on screen
testcases\comprehensive_edgecases.exe > ..\testcases\%LOG_FILE% 2>&1
cd ..\testcases

:: Print the log file contents to screen
type %LOG_FILE%

echo.
echo ========================================
echo Edge Case Test Run Complete.
echo Report generated at: %LOG_FILE%
echo ========================================
pause
