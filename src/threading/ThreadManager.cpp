#include "threading/ThreadManager.hpp"

namespace VehicleSystem {

ThreadManager::ThreadManager(std::vector<std::unique_ptr<Sensor>>& sensors,
              AlertManager& alertManager,
              DTCManager& dtcManager,
              Dashboard& dashboard,
              VehicleStatistics& stats,
              EventLogger& logger,
              Watchdog& watchdog,
              std::shared_ptr<DriverProfile> activeProfile)
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

void ThreadManager::updateProfile(std::shared_ptr<DriverProfile> newProfile) {
    std::lock_guard<std::mutex> lock(m_sensorMutex);
    m_activeProfile = newProfile;
    
    // Update alert manager dynamically
    m_alertManager.setSpeedLimit(newProfile->getSpeedLimit());
    m_alertManager.setEngineTempCritical(newProfile->getEngineTempCritical());
    m_alertManager.setEngineTempWarning(newProfile->getEngineTempWarning());
}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
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

void ThreadManager::stop() {
    if (!m_running.load()) return;
    m_running.store(false);

    // Join all threads (RAII - wait for completion)
    if (m_sensorThread.joinable()) m_sensorThread.join();
    if (m_monitoringThread.joinable()) m_monitoringThread.join();
    if (m_dashboardThread.joinable()) m_dashboardThread.join();
    if (m_loggerThread.joinable()) m_loggerThread.join();
}

std::atomic<bool>& ThreadManager::getGlobalRunningFlag() {
    static std::atomic<bool> globalRunning{true};
    return globalRunning;
}

void ThreadManager::sensorUpdaterLoop() {
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

void ThreadManager::monitoringLoop() {
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
                        m_activeProfile->getName());
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

void ThreadManager::dashboardLoop() {
    while (m_running.load()) {
        try {
            {
                std::lock_guard<std::mutex> lock(m_sensorMutex);
                m_dashboard.display(m_sensors, m_alertManager, m_dtcManager,
                                   m_stats, m_watchdog, m_activeProfile->getName());
            }
            m_watchdog.heartbeat("DashboardRenderer");
        } catch (const std::exception& e) {
            m_logger.logEvent(AlertSeverity::WARNING,
                std::string("Dashboard error: ") + e.what(), "DashboardRenderer");
        }
        Utils::sleepMs(m_dashboardIntervalMs);
    }
}

void ThreadManager::loggerLoop() {
    int perfCounter = 0;
    while (m_running.load()) {
        try {
            m_logger.processQueue();
            
            // Log Performance & DTC Snapshot every ~5 seconds (5 * 1000ms interval)
            if (++perfCounter >= 5) {
                perfCounter = 0;
                m_logger.logPerformance(m_stats.getFormattedStats());
                
                if (m_dtcManager.getActiveDTCCount() > 0) {
                    m_logger.logDTC("ACTIVE DTCs SNAPSHOT:\n" + m_dtcManager.getFormattedDTCList());
                }
            }
            
            m_watchdog.heartbeat("EventLogger");
        } catch (const std::exception& e) {
            std::cerr << "Logger error: " << e.what() << std::endl;
        }
        Utils::sleepMs(m_loggerIntervalMs);
    }
    // Final flush on shutdown
    m_logger.logPerformance(m_stats.getFormattedStats());
    m_logger.processQueue();
    m_logger.flush();
}

} // namespace VehicleSystem
