#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <functional>
#include <iostream>
#include <csignal>
#include "common/Types.hpp"
#include "common/Utils.hpp"
#include "sensors/Sensor.hpp"
#include "sensors/EngineTemperatureSensor.hpp"
#include "sensors/BatterySensor.hpp"
#include "sensors/SpeedSensor.hpp"
#include "sensors/TirePressureSensor.hpp"
#include "sensors/DoorSensor.hpp"
#include "sensors/SeatbeltSensor.hpp"
#include "alerts/AlertManager.hpp"
#include "alerts/DTC.hpp"
#include "dashboard/Dashboard.hpp"
#include "dashboard/VehicleStatistics.hpp"
#include "logging/EventLogger.hpp"
#include "threading/Watchdog.hpp"
#include "config/ConfigManager.hpp"
#include "config/DriverProfile.hpp"

namespace VehicleSystem {

/**
 * @brief Manages the 4 concurrent threads + watchdog for the vehicle system.
 * 
 * Thread 1: Sensor Updater (periodic sensor value updates)
 * Thread 2: Monitoring/Alert Evaluator (checks conditions, generates alerts)
 * Thread 3: Dashboard Display (renders console output)
 * Thread 4: Event Logger (async log writing)
 * 
 * Demonstrates: std::thread, std::mutex, std::lock_guard,
 * std::atomic, std::condition_variable, RAII thread management,
 * graceful shutdown, signal handling.
 * Maps to Adaptive AUTOSAR Execution Contexts.
 * 
 * @author Member 4 (Threading Expert)
 */
class ThreadManager {
public:
    /**
     * @brief Construct the thread manager with all subsystem references.
     */
    ThreadManager(std::vector<std::unique_ptr<Sensor>>& sensors,
                  AlertManager& alertManager,
                  DTCManager& dtcManager,
                  Dashboard& dashboard,
                  VehicleStatistics& stats,
                  EventLogger& logger,
                  Watchdog& watchdog,
                  std::shared_ptr<DriverProfile> activeProfile);

    /**
     * @brief Switch driver profile at runtime
     * Demonstrates: Thread-safe dynamic configuration update via shared_ptr
     */
    void updateProfile(std::shared_ptr<DriverProfile> newProfile);

    /**
     * @brief RAII destructor - ensures clean shutdown
     */
    ~ThreadManager();

    // Delete copy (thread ownership)
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

    /**
     * @brief Start all 4 worker threads + register with watchdog.
     */
    void start();

    /**
     * @brief Graceful shutdown - signals all threads to stop and joins them.
     */
    void stop();

    /** @brief Check if threads are running */
    bool isRunning() const { return m_running.load(); }

    /** @brief Get reference to the running flag for signal handling */
    static std::atomic<bool>& getGlobalRunningFlag();

    /** @brief Set update intervals */
    void setSensorInterval(int ms) { m_sensorUpdateIntervalMs = ms; }
    void setMonitoringInterval(int ms) { m_monitoringIntervalMs = ms; }
    void setDashboardInterval(int ms) { m_dashboardIntervalMs = ms; }
    void setLoggerInterval(int ms) { m_loggerIntervalMs = ms; }

private:
    /**
     * @brief Thread 1: Sensor Updater
     * Periodically updates all sensor values via polymorphic call.
     */
    void sensorUpdaterLoop();

    /**
     * @brief Thread 2: Monitoring/Alert Evaluation
     * Checks alert conditions and generates DTCs.
     */
    void monitoringLoop();

    /**
     * @brief Thread 3: Dashboard Display
     * Periodically refreshes the console display.
     */
    void dashboardLoop();

    /**
     * @brief Thread 4: Event Logger
     * Processes the log queue and writes to file.
     */
    void loggerLoop();

    // References to subsystems (non-owning)
    std::vector<std::unique_ptr<Sensor>>& m_sensors;
    AlertManager& m_alertManager;
    DTCManager& m_dtcManager;
    Dashboard& m_dashboard;
    VehicleStatistics& m_stats;
    EventLogger& m_logger;
    Watchdog& m_watchdog;
    std::shared_ptr<DriverProfile> m_activeProfile; // Passed by value to allow safe thread updates

    // Thread management
    std::atomic<bool> m_running;
    std::thread m_sensorThread;
    std::thread m_monitoringThread;
    std::thread m_dashboardThread;
    std::thread m_loggerThread;

    // Shared mutex for sensor data access
    std::mutex m_sensorMutex;

    // Configurable intervals
    int m_sensorUpdateIntervalMs;
    int m_monitoringIntervalMs;
    int m_dashboardIntervalMs;
    int m_loggerIntervalMs;
};

} // namespace VehicleSystem
