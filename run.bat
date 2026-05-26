@echo off
echo ========================================
echo Running Vehicle Monitoring System
echo ========================================

:: Check if build exists
if not exist "build\VehicleMonitoringSystem.exe" (
    echo [ERROR] Executable not found. Please run build.bat first
    pause
    exit /b 1
)

:: Create logs directory if doesn't exist
if not exist "logs" mkdir logs

:: Run the application
cd build
VehicleMonitoringSystem.exe

echo.
echo ========================================
echo Application terminated
echo Check logs\ directory for event logs
echo ========================================
pause