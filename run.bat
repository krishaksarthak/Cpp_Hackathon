@echo off
setlocal
echo ========================================
echo Building and Running Vehicle Monitoring System
echo Compiler: MinGW GCC
echo Build System: CMake
echo ========================================

:: Check if CMake is available
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] CMake is not installed or not in PATH.
    pause
    exit /b 1
)

:: Check if mingw32-make is available
where mingw32-make >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] mingw32-make is not installed or not in PATH.
    pause
    exit /b 1
)

:: Create build directory
if not exist "build" mkdir build
cd build

:: Create logs directory if it doesn't exist
if not exist "..\logs" mkdir "..\logs"

echo.
echo [1/3] Configuring CMake...
echo [INFO] Saving detailed output to logs\build.log
cmake -G "MinGW Makefiles" .. > "..\logs\build.log" 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed. Check logs\build.log
    cd ..
    pause
    exit /b 1
)

echo.
echo [2/3] Building Project...
mingw32-make -j4 >> "..\logs\build.log" 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Build failed. Check logs\build.log
    cd ..
    pause
    exit /b 1
)

echo.
echo [3/3] Running Tests...
ctest -V >> "..\logs\build.log" 2>&1

echo.
echo ========================================
echo Launching Application
echo ========================================
echo.

:: Return to root directory so that data/ and logs/ folders resolve correctly
cd ..

:: Run the application
build\VehicleMonitoringSystem.exe

echo.
echo ========================================
echo Application terminated
echo Check logs/ directory for event logs
echo ========================================
pause