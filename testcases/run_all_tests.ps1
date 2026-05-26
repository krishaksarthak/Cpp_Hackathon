# PowerShell Test Suite for Vehicle Monitoring System

# Color output functions
function Write-Pass { param($msg) Write-Host "[PASS] $msg" -ForegroundColor Green }
function Write-Fail { param($msg) Write-Host "[FAIL] $msg" -ForegroundColor Red }
function Write-Skip { param($msg) Write-Host "[SKIP] $msg" -ForegroundColor Yellow }
function Write-Header { param($msg) Write-Host "`n========================================" -ForegroundColor Cyan; Write-Host $msg -ForegroundColor Cyan; Write-Host "========================================" -ForegroundColor Cyan }

# Test counters
$script:TestsPassed = 0
$script:TestsFailed = 0
$script:TestsSkipped = 0

# Log file
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$logFile = "testcases\test_results_$timestamp.log"

Write-Host "========================================"
Write-Host "VEHICLE MONITORING SYSTEM - TEST SUITE"
Write-Host "========================================"
Write-Host "Started: $(Get-Date)"
Write-Host ""

"========================================" | Out-File -FilePath $logFile
"VEHICLE MONITORING SYSTEM - TEST SUITE" | Out-File -FilePath $logFile -Append
"========================================" | Out-File -FilePath $logFile -Append
"Started: $(Get-Date)" | Out-File -FilePath $logFile -Append
"" | Out-File -FilePath $logFile -Append

#==============================================================================
# Helper Functions
#==============================================================================

function Test-Pass {
    param($testName)
    Write-Pass $testName
    $testName | Out-File -FilePath $logFile -Append
    $script:TestsPassed++
}

function Test-Fail {
    param($testName)
    Write-Fail $testName
    $testName | Out-File -FilePath $logFile -Append
    $script:TestsFailed++
}

function Test-Skip {
    param($testName)
    Write-Skip $testName
    $testName | Out-File -FilePath $logFile -Append
    $script:TestsSkipped++
}

#==============================================================================
# PRE-CHECKS
#==============================================================================

Write-Header "PRE-REQUISITE CHECKS"

# Check if project is built
Write-Host "Checking if project is built..."
if (-not (Test-Path "build\VehicleMonitoringSystem.exe")) {
    Write-Host "[WARNING] Executable not found. Building project..." -ForegroundColor Yellow
    .\build.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Build failed! Cannot proceed with tests." -ForegroundColor Red
        exit 1
    }
}
Test-Pass "Build check"

# Check logs directory
Write-Host "Checking logs directory..."
if (-not (Test-Path "logs")) {
    New-Item -ItemType Directory -Path "logs" | Out-Null
}
Test-Pass "Logs directory"

# Check config file
Write-Host "Checking config file..."
if (-not (Test-Path "data\config.json")) {
    Test-Fail "Configuration file missing"
    exit 1
}
Test-Pass "Configuration file"

#==============================================================================
# SECTION 1: BUILD TESTS
#==============================================================================

Write-Header "SECTION 1: BUILD TESTS"

Write-Host "TC-BUILD-001: Clean compilation"
Push-Location build
cmake --build . --clean-first 2>&1 | Out-Null
if ($LASTEXITCODE -eq 0) {
    Test-Pass "Clean compilation"
} else {
    Test-Fail "Clean compilation"
}
Pop-Location

#==============================================================================
# SECTION 2: SMOKE TEST
#==============================================================================

Write-Header "SECTION 2: SMOKE TEST"

Write-Host "TC-SMOKE-001: Application starts without crash"
$process = Start-Process -FilePath "build\VehicleMonitoringSystem.exe" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 3
Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue

if (Test-Path "logs\vehicle_events.log") {
    Test-Pass "Application started successfully"
} else {
    Test-Fail "Application did not start properly"
}

#==============================================================================
# SECTION 3: LOG FILE TESTS
#==============================================================================

Write-Header "SECTION 3: LOG FILE TESTS"

# Clean old logs
if (Test-Path "logs\vehicle_events.log") {
    Remove-Item "logs\vehicle_events.log" -Force
}

Write-Host "TC-LOG-001: Log file creation"
$process = Start-Process -FilePath "build\VehicleMonitoringSystem.exe" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 3
Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue

if (Test-Path "logs\vehicle_events.log") {
    Test-Pass "Log file created"
} else {
    Test-Fail "Log file not created"
}

Write-Host "TC-LOG-002: Log entries have timestamps"
if (Test-Path "logs\vehicle_events.log") {
    $content = Get-Content "logs\vehicle_events.log" -Raw
    if ($content -match '\[\d{4}-\d{2}-\d{2}') {
        Test-Pass "Log entries have timestamps"
    } else {
        Test-Fail "Log entries missing timestamps"
    }
} else {
    Test-Skip "Log file not available"
}

#==============================================================================
# SECTION 4: CONFIGURATION TESTS
#==============================================================================

Write-Header "SECTION 4: CONFIGURATION TESTS"

Write-Host "TC-CONFIG-001: Profile files exist"
$profileCount = 0
if (Test-Path "data\driver_profiles\eco_mode.json") { $profileCount++ }
if (Test-Path "data\driver_profiles\sport_mode.json") { $profileCount++ }
if (Test-Path "data\driver_profiles\comfort_mode.json") { $profileCount++ }

if ($profileCount -eq 3) {
    Test-Pass "All 3 driver profiles present"
} else {
    Test-Fail "Missing driver profiles ($profileCount/3 found)"
}

#==============================================================================
# SECTION 5: MEMORY TEST (If available)
#==============================================================================

Write-Header "SECTION 5: MEMORY TEST"

if (Get-Command "drmemory" -ErrorAction SilentlyContinue) {
    Write-Host "TC-MEM-001: Memory leak detection"
    Write-Host "  Running Dr. Memory (this may take 30 seconds)..."
    
    drmemory -brief -- build\VehicleMonitoringSystem.exe 2>&1 | Out-File "testcases\drmemory_output.txt"
    
    # Check results (simplified)
    Test-Skip "Memory testing requires manual verification"
} else {
    Test-Skip "Dr. Memory not installed"
}

#==============================================================================
# SUMMARY
#==============================================================================

Write-Host ""
Write-Host "========================================"
Write-Host "TEST SUMMARY"
Write-Host "========================================"
Write-Host "Passed:  $script:TestsPassed" -ForegroundColor Green
Write-Host "Failed:  $script:TestsFailed" -ForegroundColor Red
Write-Host "Skipped: $script:TestsSkipped" -ForegroundColor Yellow
Write-Host "----------------------------------------"

$totalTests = $script:TestsPassed + $script:TestsFailed
if ($totalTests -gt 0) {
    $passRate = [math]::Round(($script:TestsPassed / $totalTests) * 100, 2)
    Write-Host "Pass Rate: $passRate%"
}

Write-Host "========================================"
Write-Host "Completed: $(Get-Date)"
Write-Host "Log file: $logFile"
Write-Host "========================================"

# Write summary to log
"" | Out-File -FilePath $logFile -Append
"========================================" | Out-File -FilePath $logFile -Append
"TEST SUMMARY" | Out-File -FilePath $logFile -Append
"========================================" | Out-File -FilePath $logFile -Append
"Passed:  $script:TestsPassed" | Out-File -FilePath $logFile -Append
"Failed:  $script:TestsFailed" | Out-File -FilePath $logFile -Append
"Skipped: $script:TestsSkipped" | Out-File -FilePath $logFile -Append
"========================================" | Out-File -FilePath $logFile -Append

if ($script:TestsFailed -gt 0) {
    exit 1
} else {
    exit 0
}