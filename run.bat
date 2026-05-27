@echo off
setlocal
:: ANSI Color Setup
for /F %%a in ('echo prompt $E ^| cmd') do set ESC=%%a
set GREEN=%ESC%[32m
set RED=%ESC%[31m
set YELLOW=%ESC%[33m
set CYAN=%ESC%[36m
set BLUE=%ESC%[94m
set RESET=%ESC%[0m

echo %CYAN%========================================%RESET%
echo Building and Running Vehicle Monitoring System
echo Compiler: MinGW GCC
echo Build System: CMake
echo %CYAN%========================================%RESET%

:: Check if CMake is available
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo %RED%[ERROR]%RESET% CMake is not installed or not in PATH.
    pause
    exit /b 1
)

:: Check if mingw32-make is available
where mingw32-make >nul 2>nul
if %errorlevel% neq 0 (
    echo %RED%[ERROR]%RESET% mingw32-make is not installed or not in PATH.
    pause
    exit /b 1
)

:: Create build directory
if not exist "build" mkdir build
cd build

:: Create logs directory if it doesn't exist
if not exist "..\logs" mkdir "..\logs"

echo.
echo %YELLOW%[1/3]%RESET% Configuring CMake...
echo %BLUE%[INFO]%RESET% Saving detailed output to logs\build.log
cmake -G "MinGW Makefiles" .. > "..\logs\build.log" 2>&1
if %errorlevel% neq 0 (
    echo %RED%[ERROR]%RESET% CMake configuration failed. Check logs\build.log
    cd ..
    pause
    exit /b 1
)

echo.
echo %YELLOW%[2/3]%RESET% Building Project...
mingw32-make -j4 >> "..\logs\build.log" 2>&1
if %errorlevel% neq 0 (
    echo %RED%[ERROR]%RESET% Build failed. Check logs\build.log
    cd ..
    pause
    exit /b 1
)

echo.
echo %YELLOW%[3/3]%RESET% Running Tests...
ctest -V >> "..\logs\build.log" 2>&1

echo.
echo %CYAN%========================================%RESET%
echo Launching Application
echo %CYAN%========================================%RESET%
echo.

:: Return to root directory so that data/ and logs/ folders resolve correctly
cd ..

:: Run the application
build\VehicleMonitoringSystem.exe

echo.
echo %CYAN%========================================%RESET%
echo Application terminated
echo Check logs/ directory for event logs
echo %CYAN%========================================%RESET%
pause