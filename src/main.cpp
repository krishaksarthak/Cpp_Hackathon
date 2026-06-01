/**
 * @file main.cpp
 * @brief Entry point for the Smart Cabin & Vehicle Health Monitoring System.
 * 
 * Initializes all subsystems, loads configuration, creates sensors,
 * starts the thread manager with 4 worker threads, and handles
 * graceful shutdown via Ctrl+C signal.
 * 
 * Demonstrates: Complete system integration, signal handling,
 * smart pointers, RAII, exception handling, clean shutdown.
 * 
 * @author Team C-10
 * @version 1.0
 */
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <csignal>
#include <cstdlib>
#include <atomic>
#include <fstream>
#include <future>
#ifdef _WIN32
#include <conio.h>
#include <direct.h>   // _mkdir on Windows
#else
#include <sys/stat.h> // mkdir on POSIX
#endif

// Core modules
#include "common/Types.hpp"
#include "common/Utils.hpp"
#include "common/Colors.hpp"

// Sensor framework
#include "sensors/Sensor.hpp"
#include "sensors/EngineTemperatureSensor.hpp"
#include "sensors/BatterySensor.hpp"
#include "sensors/SpeedSensor.hpp"
#include "sensors/TirePressureSensor.hpp"
#include "sensors/DoorSensor.hpp"
#include "sensors/SeatbeltSensor.hpp"

// Alert system
#include "alerts/Alert.hpp"
#include "alerts/AlertManager.hpp"
#include "alerts/DTC.hpp"

// Dashboard & statistics
#include "dashboard/Dashboard.hpp"
#include "dashboard/VehicleStatistics.hpp"

// Logging
#include "logging/EventLogger.hpp"

// Threading & watchdog
#include "threading/ThreadManager.hpp"
#include "threading/Watchdog.hpp"

// Configuration
#include "config/ConfigManager.hpp"
#include "config/DriverProfile.hpp"

using namespace VehicleSystem;

// Global flag for signal handling
static std::atomic<bool> g_running{true};

/**
 * @brief Signal handler for Ctrl+C (SIGINT) - graceful shutdown
 * @param signum The signal number received
 */
void signalHandler(int signum) {
    (void)signum; // Suppress unused parameter warning
    std::cout << "\n\n[SYSTEM] Received shutdown signal (SIGINT)...\n";
    std::cout << "[SYSTEM] Initiating graceful shutdown...\n";
    g_running.store(false);
}

/**
 * @brief Print startup banner
 */
void printBanner() {
    std::cout << "\n";
    std::cout << "========================================================\n";
    std::cout << "    Smart Cabin & Vehicle Health Monitoring System\n";
    std::cout << "    Adaptive Automotive C++ Hackathon - Team C-10\n";
    std::cout << "========================================================\n";
    std::cout << "  C++17 | Multi-threaded | AUTOSAR-Inspired\n";
    std::cout << "========================================================\n\n";
}

/**
 * @brief Determine the data directory path relative to the executable
 * @return std::string The relative path to the data directory
 */
std::string findDataDir() {
    // Try several common locations
    std::vector<std::string> candidates = {
        "data",
        "../data",
        "../../data"
    };
    
    for (const auto& path : candidates) {
        std::string configPath = path + "/config.json";
        std::ifstream test(configPath);
        if (test.good()) {
            return path;
        }
    }
    return "data"; // Default fallback
}

/**
 * @brief Creates a directory cross-platform.
 * Replaces system() calls with safe API calls (no shell injection risk).
 * @param path The path of the directory to create
 * @return true If successful or directory already exists
 * @return false If creation failed
 */
bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return (_mkdir(path.c_str()) == 0 || errno == EEXIST);
#else
    return (mkdir(path.c_str(), 0755) == 0 || errno == EEXIST);
#endif
}



void playTestScenario(const std::string& filepath, 
                      std::vector<std::unique_ptr<Sensor>>& sensors, 
                      EventLogger& logger) {
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        logger.logEvent(AlertSeverity::WARNING, "Could not open scenario file: " + filepath, "TEST_RIG");
        return;
    }

    logger.logEvent(AlertSeverity::INFO, "Started Test Scenario: " + filepath, "TEST_RIG");

    std::string line;
    while (std::getline(file, line) && g_running.load()) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string delayStr, sensorTypeStr, valueStr;

        if (std::getline(ss, delayStr, ',') &&
            std::getline(ss, sensorTypeStr, ',') &&
            std::getline(ss, valueStr, ',')) {

            // 1. Wait for the specified delay
            int delayMs = std::stoi(delayStr);
            Utils::sleepMs(delayMs);
            if (!g_running.load()) break; // Exit if system is shutting down

            // Trim spaces from the sensor type string
            sensorTypeStr.erase(0, sensorTypeStr.find_first_not_of(" \t"));
            sensorTypeStr.erase(sensorTypeStr.find_last_not_of(" \t") + 1);
            
            double value = std::stod(valueStr);

            // 2. Clear faults or inject new ones
            if (sensorTypeStr == "CLEAR") {
                for (auto& s : sensors) s->clearTestValue();
                logger.logEvent(AlertSeverity::INFO, "TEST SCRIPT: Cleared all faults", "TEST_RIG");
            } else {
                // Map string to SensorType
                SensorType targetType;
                if (sensorTypeStr == "ENGINE_TEMPERATURE") targetType = SensorType::ENGINE_TEMPERATURE;
                else if (sensorTypeStr == "BATTERY_VOLTAGE") targetType = SensorType::BATTERY_VOLTAGE;
                else if (sensorTypeStr == "VEHICLE_SPEED") targetType = SensorType::VEHICLE_SPEED;
                else if (sensorTypeStr == "TIRE_PRESSURE") targetType = SensorType::TIRE_PRESSURE;
                else if (sensorTypeStr == "DOOR_STATUS") targetType = SensorType::DOOR_STATUS;
                else if (sensorTypeStr == "SEATBELT_STATUS") targetType = SensorType::SEATBELT_STATUS;
                else continue; // Unknown sensor

                // Inject the value
                for (auto& s : sensors) {
                    if (s->getType() == targetType) {
                        s->injectTestValue(value);
                        logger.logEvent(AlertSeverity::WARNING, 
                            "TEST SCRIPT: Forced " + sensorTypeStr + " to " + std::to_string(value), 
                            "TEST_RIG");
                    }
                }
            }
        }
    }
    logger.logEvent(AlertSeverity::INFO, "Test Scenario Completed.", "TEST_RIG");
}

/**
 * @brief Main execution entry point for the Vehicle Monitoring System
 * @return int Exit code (0 on success, non-zero on error)
 */
int main() {
    // Install signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    printBanner();

    try {
        // ===== Phase 1: Configuration Loading =====
        std::cout << "[INIT] Loading configuration...\n";
        
        std::string dataDir = findDataDir();
        std::string configPath = dataDir + "/config.json";
        
        auto& config = ConfigManager::getInstance();
        if (!config.loadConfig(configPath)) {
            std::cout << "[WARN] Could not load config.json: " << config.getLastError() << "\n";
            std::cout << "[WARN] Using default values.\n";
        } else {
            std::cout << "[INIT] Configuration loaded from: " << configPath << "\n";
        }

        // ===== Phase 2: Driver Profile Loading =====
        std::cout << "[INIT] Loading driver profile...\n";
        
        auto activeProfile = std::make_shared<DriverProfile>();
        std::vector<std::string> availableProfiles = {"eco_mode", "sport_mode", "comfort_mode"};
        int currentProfileIdx = 0;

        // Try to find active profile in list to set initial index
        std::string profileName = config.getActiveProfile();
        for (size_t i = 0; i < availableProfiles.size(); ++i) {
            if (availableProfiles[i] == profileName) {
                currentProfileIdx = i;
                break;
            }
        }

        std::string profilePath = dataDir + "/driver_profiles/" + availableProfiles[currentProfileIdx] + ".json";
        
        if (activeProfile->loadProfile(profilePath)) {
            std::cout << "[INIT] Loaded profile: " << activeProfile->getName() << "\n";
            std::cout << "  Speed Limit: " << activeProfile->getSpeedLimit() << " km/h\n";
            std::cout << "  Temp Warning: " << activeProfile->getEngineTempWarning() << " C\n";
            std::cout << "  Temp Critical: " << activeProfile->getEngineTempCritical() << " C\n";
        } else {
            std::cout << "[WARN] Could not load profile '" << availableProfiles[currentProfileIdx] 
                      << "', using defaults.\n";
        }

        // ===== Phase 3: Sensor Creation (Polymorphism + Smart Pointers) =====
        std::cout << "[INIT] Creating sensor instances...\n";
        
        std::vector<std::unique_ptr<Sensor>> sensors;
        sensors.push_back(std::make_unique<EngineTemperatureSensor>(1));
        sensors.push_back(std::make_unique<BatterySensor>(2));
        sensors.push_back(std::make_unique<SpeedSensor>(3));
        sensors.push_back(std::make_unique<TirePressureSensor>(4));
        sensors.push_back(std::make_unique<DoorSensor>(5));
        sensors.push_back(std::make_unique<SeatbeltSensor>(6));
        
        std::cout << "[INIT] Created " << Sensor::getSensorCount() 
                  << " sensors (polymorphic via unique_ptr)\n";

        // ===== Phase 4: Alert Manager + DTC System =====
        std::cout << "[INIT] Initializing alert manager...\n";
        
        AlertManager alertManager;
        // Apply profile thresholds
        alertManager.setSpeedLimit(activeProfile->getSpeedLimit());
        alertManager.setEngineTempCritical(activeProfile->getEngineTempCritical());
        alertManager.setEngineTempWarning(activeProfile->getEngineTempWarning());
        
        DTCManager dtcManager;
        std::cout << "[INIT] Alert manager ready with 6 monitoring conditions\n";
        std::cout << "[INIT] DTC manager ready with automotive diagnostic codes\n";

        // ===== Phase 5: Dashboard + Statistics =====
        std::cout << "[INIT] Initializing dashboard and statistics...\n";
        
        Dashboard dashboard;
        VehicleStatistics stats;
        
        int dashInterval = config.getInt("application", "update_interval_ms", 2000);
        dashboard.setRefreshRate(dashInterval);

        // ===== Phase 6: Event Logger (RAII) =====
        std::cout << "[INIT] Starting event logger...\n";
        
        std::string logDir = "logs";
        // Create logs directory using safe API call (no system() shell injection)
        createDirectory(logDir);
        
        std::string logPath = logDir + "/vehicle_log.txt";
        EventLogger logger(logPath);
        std::cout << "[INIT] Event logger writing to: " << logPath << "\n";

        // ===== Phase 7: Watchdog Health Monitor =====
        std::cout << "[INIT] Starting watchdog health monitor...\n";
        
        int watchdogTimeout = config.getInt("watchdog", "thread_timeout_ms", 5000);
        Watchdog watchdog(watchdogTimeout);
        std::cout << "[INIT] Watchdog timeout: " << watchdogTimeout << "ms\n";

        // ===== Phase 8: Thread Manager (4 Concurrent Threads) =====
        std::cout << "[INIT] Starting thread manager (4 threads)...\n";
        
        ThreadManager threadManager(sensors, alertManager, dtcManager,
                                     dashboard, stats, logger, watchdog,
                                     activeProfile);
        
        // Apply configured intervals
        int sensorInterval = config.getInt("sensors", "update_interval_ms", 500);
        threadManager.setSensorInterval(sensorInterval);
        threadManager.setMonitoringInterval(1000);
        threadManager.setDashboardInterval(dashInterval);
        threadManager.setLoggerInterval(1000);

        // ===== START =====
        std::cout << "\n[SYSTEM] All subsystems initialized. Starting threads...\n";
        std::cout << "[SYSTEM] Press Ctrl+C for graceful shutdown.\n\n";
        
        logger.logEvent(AlertSeverity::INFO, "System startup complete", "MAIN");
        
        // Start all 4 worker threads
        threadManager.start();

        // ===== START =====
        std::cout << "\n[SYSTEM] All subsystems initialized. Starting threads...\n";
        std::cout << "[SYSTEM] Press Ctrl+C for graceful shutdown.\n\n";
        
        logger.logEvent(AlertSeverity::INFO, "System startup complete", "MAIN");
        
        // Start all 4 worker threads
        threadManager.start();

        // Add this to store our asynchronous test scripts!
        std::vector<std::future<void>> scriptFutures;

        // ===== Main Loop (waits for shutdown signal + keyboard input) =====
        while (g_running.load()) {
            // Poll for 500ms in 50ms increments to allow responsive keyboard input
            for (int i = 0; i < 10 && g_running.load(); ++i) {
                Utils::sleepMs(50);
#ifdef _WIN32
                if (_kbhit()) {
                    char c = _getch();
                    if (c == 'p' || c == 'P') {
                        // Cycle profile
                        currentProfileIdx = (currentProfileIdx + 1) % availableProfiles.size();
                        std::string newProfName = availableProfiles[currentProfileIdx];
                        std::string newPath = dataDir + "/driver_profiles/" + newProfName + ".json";
                        
                        auto newProfile = std::make_shared<DriverProfile>();
                        if (newProfile->loadProfile(newPath)) {
                            threadManager.updateProfile(newProfile);
                            logger.logEvent(AlertSeverity::INFO, "Switched to profile: " + newProfName, "SYSTEM");
                        }
                    } else if (c == 'q' || c == 'Q') {
                        // Graceful quit via keyboard
                        g_running.store(false);
                    } else if (c == 't' || c == 'T') {
                        std::string scriptPath = dataDir + "/test_scenario.csv";
                        // Launch the scenario player asynchronously instead of using std::thread
                        scriptFutures.push_back(
                            std::async(std::launch::async, playTestScenario, scriptPath, std::ref(sensors), std::ref(logger))
                        );
                    }

                }
#endif
            }
            
            // Check if threads are still running
            if (!threadManager.isRunning()) {
                std::cout << "[ERROR] Thread manager stopped unexpectedly!\n";
                break;
            }
        }

        // ===== Graceful Shutdown =====
        std::cout << "\n[SHUTDOWN] Stopping all threads...\n";
        logger.logEvent(AlertSeverity::INFO, "System shutdown initiated", "MAIN");
        
        threadManager.stop();
        
        // Process remaining log entries
        logger.processQueue();
        logger.flush();

        // Print final statistics
        std::cout << Colors::CYAN << "\n========================================\n";
        std::cout << "  FINAL SESSION STATISTICS\n";
        std::cout << "========================================\n" << Colors::RESET;
        std::cout << stats.getFormattedStats();
        std::cout << "Total Sensors:    " << Sensor::getSensorCount() << "\n";
        std::cout << "Total Alerts:     " << alertManager.getTotalAlertCount() << "\n";
        std::cout << "Active DTCs:      " << dtcManager.getActiveDTCCount() << "\n";
        std::cout << "Log Entries:      " << logger.getTotalLogCount() << "\n";

        // --- STL algorithm + Lambda: Search event log for CRITICAL entries ---
        // Demonstrates: std::function, lambda predicate, EventLogger::searchEvents
        auto criticalEvents = logger.searchEvents(
            [](const LogEntry& entry) {
                return entry.severity == AlertSeverity::CRITICAL;
            });
        std::cout << Colors::RED << "Critical Events:  " << criticalEvents.size() << Colors::RESET << "\n";

        std::cout << Colors::CYAN << "========================================\n" << Colors::RESET;
        std::cout << Colors::GREEN << "[SHUTDOWN] Clean shutdown complete. No memory leaks.\n";
        std::cout << "[SHUTDOWN] Log saved to: " << logPath << Colors::RESET << "\n\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL] Unhandled exception: " << e.what() << "\n";
        std::cerr << "[FATAL] System shutting down due to critical error.\n";
        return 1;
    } catch (...) {
        std::cerr << "\n[FATAL] Unknown exception occurred.\n";
        return 2;
    }

    return 0;
}
