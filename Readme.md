# Adaptive Automotive C++ Hackathon ( Team C-10)
## Smart Cabin & Vehicle Health Monitoring System

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)

---

## 🚗 Project Overview

A production-grade automotive cockpit ECU simulation implementing:
- **Multi-sensor monitoring** (Engine, Battery, Speed, Tire, Door, Seatbelt)
- **Real-time alert management** with severity classification
- **Diagnostic Trouble Codes (DTC)** system
- **Thread-safe concurrent processing**
- **Driver profile management** (Eco/Sport/Comfort modes)
- **JSON-based configuration**
- **Crash-safe event logging**
- **Vehicle statistics dashboard**
- **Watchdog health monitoring**

---

## 👥 Team Members

| Name | Focus Area |
|------|------------|
| Nischal NV | Sensor Framework |
| Sarthak K  | Alert Management + DTC |
| Payel S    | UI + Logger + Statistics |
| Ayan M     | Concurrency + Watchdog |
| Priyanshu P| Config + Profiles + Docs |

---

## 🏗️ Architecture
```
┌────────────────────────────────────────┐
│     Smart Vehicle Application          │
├────────────────────────────────────────┤
│  ┌─────────────────────────────────┐   │
│  │   Dashboard Manager             │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │   Alert Manager + DTC           │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │   Sensor Framework              │   │
│  │   ├─ Engine  ├─ Battery         │   │
│  │   ├─ Speed   ├─ Tire            │   │
│  │   ├─ Door    └─ Seatbelt        │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │   Event Logger + Statistics     │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │   Thread Manager + Watchdog     │   │
│  └─────────────────────────────────┘   │
└────────────────────────────────────────┘
```

---

## 🚀 Quick Start

### Prerequisites
```bash
# C++17 compatible compiler
g++ --version  # GCC 7.0+ or Clang 5.0+

# CMake (optional but recommended)
cmake --version  # 3.10+

# nlohmann/json (for JSON parsing)
sudo apt-get install nlohmann-json3-dev  # Ubuntu/Debian
# or download single-header from: https://github.com/nlohmann/json
```

### Build & Run

**Windows Environment (MinGW GCC):**
Simply double-click the `run.bat` file in the root directory, or run it via the terminal:
```cmd
.\run.bat
```
*(This all-in-one script automatically configures CMake, builds the project with `mingw32-make`, runs all unit tests via `ctest`, and launches the application dashboard.)*

**Linux/Mac Environment:**
```bash
chmod +x run.sh
./run.sh
```

**Manual Build (CMake):**
```cmd
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j4
cd ..
build\VehicleMonitoringSystem.exe
```

---

## 📋 Features Implemented

### ✅ Mandatory Requirements
- [x] 6 Sensor classes with inheritance
- [x] 6 Alert conditions with enum severity
- [x] Dashboard with formatted output
- [x] Event logger with timestamps
- [x] 4 concurrent threads
- [x] Thread-safe synchronization
- [x] STL containers (vector, map, set, queue)
- [x] Smart pointers (unique_ptr, shared_ptr)
- [x] Exception handling
- [x] RAII resource management
- [x] Operator overloading (`<<`, `==`, `!=`, `<`, `>`)
- [x] Lambda expressions (event filtering, alert filtering, statistics)
- [x] Templates (`SensorDataBuffer<T>` — generic circular buffer used by VehicleStatistics)
- [x] Static Members (`Alert::s_nextId`, `Sensor::s_sensorCount`)
- [x] Copy/Move Semantics (Alert, Sensor, SensorDataBuffer, JsonValue)

### 🌟 Bonus Features
- [x] JSON configuration management
- [x] Driver profile system (Eco/Sport/Comfort)
- [x] Real-Time Interactive Profile Switching ('p' hotkey)
- [x] Vehicle statistics dashboard
- [x] DTC (Diagnostic Trouble Code) system
- [x] Watchdog health monitor
- [x] Crash-safe segregated logging
- [x] Runtime config reload

---

## 🧪 Testing

We provide an exhaustive automated test suite (47+ assertions) covering system bounds, logic edge cases, concurrency, and file system crash-recoveries.

**Run the Master Test Suite (Windows CMD):**
```cmd
.\testcases\run_all_tests.bat
```

**Run the Master Test Suite (Windows PowerShell):**
```powershell
.\testcases\run_all_tests.ps1
```

**Run the Master Test Suite (Linux / macOS):**
```bash
chmod +x testcases/run_all_tests.sh
./testcases/run_all_tests.sh
```

**What the Master Test Script Does (7-Stage Pipeline):**
1. **Clean Recompilation:** Compiles the project from scratch.
2. **Smoke Test:** Boots the application and ensures it doesn't crash on startup.
3. **Log Validation:** Parses the runtime logs to verify precise timestamp formatting.
4. **Configuration Validation:** Validates `config.json` and hot-swappable driver profiles.
5. **Standard C++ Unit Tests:** Executes `test_sensors.exe`, `test_alerts.exe`, and `test_threading.exe` to evaluate bounds and logic.
6. **File System Crash Tests:** Dynamically deletes critical runtime files to verify the Watchdog gracefully falls back to defaults instead of fatal crashing.
7. **Comprehensive Edge Cases (C++ EXTREME):** Unleashes an aggressive 11-stage suite (`comprehensive_edgecases.exe`) simulating extreme concurrency thread starvation, lock contention, and physics limit violations.

**Result Reports:** 
All tests automatically calculate a final pass percentage on the terminal and generate a massive, fully-detailed report containing every executed assertion in `testcases/logs/detailed_test_report.log`.

---

## 📊 Demo Scenarios

### Scenario 1: Normal Operation
```bash
# All sensors running, no alerts
./run.sh
```

### Scenario 2: Real-Time Profile Switching
```bash
# Launch the dashboard normally
./run.sh
# Press 'p' continuously to hot-swap dynamically between Eco, Sport, and Comfort modes!
# The limits will adapt in real-time and the console will colorize.
```

### Scenario 3: Alert Generation
```bash
# Manually trigger engine overheat
# Sensor will randomly generate high temp > 110°C
```

### Scenario 4: Thread Recovery
```bash
# Watchdog restarts crashed threads automatically
```

---

## 📂 Project Structure
include/        → Header files (.hpp)
src/            → Implementation files (.cpp)
data/           → Configuration files (JSON)
logs/           → Runtime log files
tests/          → Unit tests
scripts/        → Build/test automation
docs/           → Documentation
demo/           → Screenshots & presentation

---

## 🔧 Configuration

Edit `data/config.json`:
```json
{
  "sensors": {
    "engine_temp_threshold": 110,
    "battery_voltage_min": 10.0,
    "update_interval_ms": 1000
  },
  "active_profile": "eco_mode"
}
```

---

## 📖 Documentation

- [Architecture Guide](docs/ARCHITECTURE.md)
- [Team Roles](docs/TEAM_ROLES.md)
- [Coding Standards](docs/CODING_STANDARDS.md)
- [API Reference](docs/API_REFERENCE.md)

---

## 🏆 Evaluation Criteria Coverage

| Criteria | Weightage | Status |
|----------|-----------|--------|
| C++ Design Quality | 25% | ✅ Modular, SOLID principles |
| OOP Usage | 15% | ✅ Inheritance, polymorphism |
| Modern C++ Features | 15% | ✅ C++17, smart ptrs, lambdas |
| Threading & Sync | 15% | ✅ 4 threads, mutex, lock_guard |
| STL & Templates | 10% | ✅ vector, map, set, templates |
| Code Readability | 10% | ✅ Documented, formatted |
| Innovation | 10% | ✅ 5 bonus features |

---

## 🐛 Known Issues

- None currently

---

## 📝 License

MIT License — no LICENSE file is included in this repository

---

## 🙏 Acknowledgments

- Visteon C++ Training Team
- Adaptive AUTOSAR specification
- Modern C++ best practices community

---

**Built by Team C-10**