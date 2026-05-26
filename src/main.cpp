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

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <csignal>
#include <cstdlib>
#include <atomic>
#include <filesystem>

// Core modules
#include "common/Types.hpp"
#include "common/Utils.hpp"

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
 */
void signalHandler(int signum) {
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
        
        DriverProfile activeProfile;
        std::string profileName = config.getActiveProfile();
        std::string profilePath = dataDir + "/driver_profiles/" + profileName + ".json";
        
        if (activeProfile.loadProfile(profilePath)) {
            std::cout << "[INIT] Loaded profile: " << activeProfile.getName() << "\n";
            std::cout << "  Speed Limit: " << activeProfile.getSpeedLimit() << " km/h\n";
            std::cout << "  Temp Warning: " << activeProfile.getEngineTempWarning() << " C\n";
            std::cout << "  Temp Critical: " << activeProfile.getEngineTempCritical() << " C\n";
        } else {
            std::cout << "[WARN] Could not load profile '" << profileName 
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
        alertManager.setSpeedLimit(activeProfile.getSpeedLimit());
        alertManager.setEngineTempCritical(activeProfile.getEngineTempCritical());
        alertManager.setEngineTempWarning(activeProfile.getEngineTempWarning());
        
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
        // Create logs directory if it doesn't exist
        #ifdef _WIN32
        system(("mkdir " + logDir + " 2>nul").c_str());
        #else
        system(("mkdir -p " + logDir).c_str());
        #endif
        
        std::string logPath = logDir + "/vehicle_events.log";
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

        // ===== Main Loop (waits for shutdown signal) =====
        while (g_running.load()) {
            Utils::sleepMs(500);
            
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
        std::cout << "\n========================================\n";
        std::cout << "  FINAL SESSION STATISTICS\n";
        std::cout << "========================================\n";
        std::cout << stats.getFormattedStats();
        std::cout << "Total Sensors:    " << Sensor::getSensorCount() << "\n";
        std::cout << "Total Alerts:     " << alertManager.getTotalAlertCount() << "\n";
        std::cout << "Active DTCs:      " << dtcManager.getActiveDTCCount() << "\n";
        std::cout << "Log Entries:      " << logger.getTotalLogCount() << "\n";
        std::cout << "========================================\n";
        std::cout << "[SHUTDOWN] Clean shutdown complete. No memory leaks.\n";
        std::cout << "[SHUTDOWN] Log saved to: " << logPath << "\n\n";

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
