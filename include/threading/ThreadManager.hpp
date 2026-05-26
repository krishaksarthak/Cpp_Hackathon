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
                  DriverProfile& activeProfile)
        : m_sensors(sensors),
          m_alertManager(alertManager),
          m_dtcManager(dtcManager),
          m_dashboard(dashboard),
          m_stats(stats),
          m_logger(logger),
          m_watchdog(watchdog),
          m_activeProfile(activeProfile),
          m_running(false),
          m_sensorUpdateIntervalMs(500),
          m_monitoringIntervalMs(1000),
          m_dashboardIntervalMs(2000),
          m_loggerIntervalMs(1000) {}

    /**
     * @brief RAII destructor - ensures clean shutdown
     */
    ~ThreadManager() {
        stop();
    }

    // Delete copy (thread ownership)
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

    /**
     * @brief Start all 4 worker threads + register with watchdog.
     */
    void start() {
        if (m_running.load()) return;
        m_running.store(true);

        // Register threads with watchdog
        m_watchdog.registerThread("SensorUpdater");
        m_watchdog.registerThread("MonitoringEngine");
        m_watchdog.registerThread("DashboardRenderer");
        m_watchdog.registerThread("EventLogger");

        // Launch 4 threads
        m_sensorThread = std::thread(&ThreadManager::sensorUpdaterLoop, this);
        m_monitoringThread = std::thread(&ThreadManager::monitoringLoop, this);
        m_dashboardThread = std::thread(&ThreadManager::dashboardLoop, this);
        m_loggerThread = std::thread(&ThreadManager::loggerLoop, this);

        m_logger.logEvent(AlertSeverity::INFO,
            "Thread Manager started - 4 threads active", "ThreadManager");
    }

    /**
     * @brief Graceful shutdown - signals all threads to stop and joins them.
     */
    void stop() {
        if (!m_running.load()) return;
        m_running.store(false);

        // Join all threads (RAII - wait for completion)
        if (m_sensorThread.joinable()) m_sensorThread.join();
        if (m_monitoringThread.joinable()) m_monitoringThread.join();
        if (m_dashboardThread.joinable()) m_dashboardThread.join();
        if (m_loggerThread.joinable()) m_loggerThread.join();
    }

    /** @brief Check if threads are running */
    bool isRunning() const { return m_running.load(); }

    /** @brief Get reference to the running flag for signal handling */
    static std::atomic<bool>& getGlobalRunningFlag() {
        static std::atomic<bool> globalRunning{true};
        return globalRunning;
    }

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
    void sensorUpdaterLoop() {
        while (m_running.load()) {
            try {
                {
                    std::lock_guard<std::mutex> lock(m_sensorMutex);
                    // Polymorphic update - each sensor's update() is called
                    for (auto& sensor : m_sensors) {
                        sensor->performUpdate();
                    }
                }

                // Record statistics
                for (const auto& sensor : m_sensors) {
                    switch (sensor->getType()) {
                        case SensorType::VEHICLE_SPEED:
                            m_stats.recordSpeed(sensor->getValue());
                            break;
                        case SensorType::ENGINE_TEMPERATURE:
                            m_stats.recordTemperature(sensor->getValue());
                            break;
                        case SensorType::BATTERY_VOLTAGE:
                            m_stats.recordBatteryVoltage(sensor->getValue());
                            break;
                        case SensorType::TIRE_PRESSURE:
                            m_stats.recordTirePressure(sensor->getValue());
                            break;
                        default:
                            break;
                    }
                }

                m_watchdog.heartbeat("SensorUpdater");
            } catch (const std::exception& e) {
                m_logger.logEvent(AlertSeverity::CRITICAL,
                    std::string("Sensor update error: ") + e.what(), "SensorUpdater");
            }
            Utils::sleepMs(m_sensorUpdateIntervalMs);
        }
    }

    /**
     * @brief Thread 2: Monitoring/Alert Evaluation
     * Checks alert conditions and generates DTCs.
     */
    void monitoringLoop() {
        while (m_running.load()) {
            try {
                std::vector<Alert> newAlerts;
                {
                    std::lock_guard<std::mutex> lock(m_sensorMutex);
                    newAlerts = m_alertManager.evaluateConditions(m_sensors);
                }

                // Process new alerts
                for (const auto& alert : newAlerts) {
                    m_logger.logAlert(alert);

                    // Generate DTC for each new alert
                    {
                        std::lock_guard<std::mutex> lock(m_sensorMutex);
                        m_dtcManager.generateDTC(alert, m_sensors,
                            m_activeProfile.getName());
                    }

                    // Record in statistics
                    m_stats.recordAlert(alert.getSeverity(),
                        sensorTypeToString(alert.getSource()));
                }

                // Watchdog check
                m_watchdog.checkHealth();
                auto unhealthy = m_watchdog.getUnhealthyThreads();
                for (const auto& thread : unhealthy) {
                    m_logger.logEvent(AlertSeverity::WARNING,
                        "Thread unhealthy: " + thread, "Watchdog");
                }

                m_watchdog.heartbeat("MonitoringEngine");
            } catch (const std::exception& e) {
                m_logger.logEvent(AlertSeverity::CRITICAL,
                    std::string("Monitoring error: ") + e.what(), "MonitoringEngine");
            }
            Utils::sleepMs(m_monitoringIntervalMs);
        }
    }

    /**
     * @brief Thread 3: Dashboard Display
     * Periodically refreshes the console display.
     */
    void dashboardLoop() {
        while (m_running.load()) {
            try {
                {
                    std::lock_guard<std::mutex> lock(m_sensorMutex);
                    m_dashboard.display(m_sensors, m_alertManager, m_dtcManager,
                                       m_stats, m_watchdog, m_activeProfile.getName());
                }
                m_watchdog.heartbeat("DashboardRenderer");
            } catch (const std::exception& e) {
                m_logger.logEvent(AlertSeverity::WARNING,
                    std::string("Dashboard error: ") + e.what(), "DashboardRenderer");
            }
            Utils::sleepMs(m_dashboardIntervalMs);
        }
    }

    /**
     * @brief Thread 4: Event Logger
     * Processes the log queue and writes to file.
     */
    void loggerLoop() {
        while (m_running.load()) {
            try {
                m_logger.processQueue();
                m_watchdog.heartbeat("EventLogger");
            } catch (const std::exception& e) {
                std::cerr << "Logger error: " << e.what() << std::endl;
            }
            Utils::sleepMs(m_loggerIntervalMs);
        }
        // Final flush on shutdown
        m_logger.processQueue();
        m_logger.flush();
    }

    // References to subsystems (non-owning)
    std::vector<std::unique_ptr<Sensor>>& m_sensors;
    AlertManager& m_alertManager;
    DTCManager& m_dtcManager;
    Dashboard& m_dashboard;
    VehicleStatistics& m_stats;
    EventLogger& m_logger;
    Watchdog& m_watchdog;
    DriverProfile& m_activeProfile;

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
