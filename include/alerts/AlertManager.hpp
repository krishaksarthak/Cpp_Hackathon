#pragma once

#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <memory>
#include <algorithm>
#include <functional>
#include "common/Types.hpp"
#include "alerts/Alert.hpp"
#include "sensors/Sensor.hpp"
#include "sensors/DoorSensor.hpp"
#include "sensors/SeatbeltSensor.hpp"

namespace VehicleSystem {

/**
 * @brief Manages alert detection, storage, and lifecycle.
 * 
 * Evaluates 6 critical conditions against sensor readings:
 * 1. Engine overheat (temp > threshold)
 * 2. Low battery (voltage < threshold)
 * 3. Low tire pressure (pressure < threshold)
 * 4. Overspeed (speed > limit)
 * 5. Door open while moving (door OPEN + speed > 10)
 * 6. Seatbelt unlocked while moving (unlocked + speed > 10)
 * 
 * Demonstrates: STL containers (vector, set), smart pointers,
 * lambda expressions, thread-safety, templates.
 * Maps to Vehicle Health Manager in AUTOSAR.
 * 
 * @author Member 2 (Alert Specialist)
 */
class AlertManager {
public:
    AlertManager()
        : m_speedLimit(120), m_engineTempCritical(110),
          m_engineTempWarning(100), m_batteryVoltageMin(10.0),
          m_tirePressureMin(25) {}

    ~AlertManager() = default;

    // Delete copy (shared resource)
    AlertManager(const AlertManager&) = delete;
    AlertManager& operator=(const AlertManager&) = delete;

    /**
     * @brief Evaluate all 6 alert conditions against current sensor values.
     * 
     * @param sensors Vector of sensor unique_ptrs (polymorphic access)
     * @return Vector of newly generated alerts
     */
    std::vector<Alert> evaluateConditions(
        const std::vector<std::unique_ptr<Sensor>>& sensors);

    /** @brief Get currently active alerts */
    std::vector<Alert> getActiveAlerts() const;

    /** @brief Get full alert history */
    std::vector<Alert> getAlertHistory() const;

    size_t getActiveAlertCount() const;

    size_t getTotalAlertCount() const;

    /** @brief Remove all inactive alerts from active list */
    void clearResolvedAlerts();

    /**
     * @brief Filter alerts by severity (demonstrates lambda + STL)
     */
    std::vector<Alert> filterAlerts(AlertSeverity severity) const;

    // --- Configurable thresholds (for driver profiles) ---
    void setSpeedLimit(int limit) { std::lock_guard<std::mutex> lock(m_configMutex); m_speedLimit = limit; }
    void setEngineTempCritical(int temp) { std::lock_guard<std::mutex> lock(m_configMutex); m_engineTempCritical = temp; }
    void setEngineTempWarning(int temp) { std::lock_guard<std::mutex> lock(m_configMutex); m_engineTempWarning = temp; }
    void setBatteryVoltageMin(double voltage) { std::lock_guard<std::mutex> lock(m_configMutex); m_batteryVoltageMin = voltage; }
    void setTirePressureMin(int pressure) { std::lock_guard<std::mutex> lock(m_configMutex); m_tirePressureMin = pressure; }

    int getSpeedLimit() const { std::lock_guard<std::mutex> lock(m_configMutex); return m_speedLimit; }
    int getEngineTempCritical() const { std::lock_guard<std::mutex> lock(m_configMutex); return m_engineTempCritical; }

private:
    /** @brief Check if an active alert with given message prefix exists */
    bool hasActiveAlert(const std::string& prefix) const;

    /** @brief Resolve (deactivate) alerts matching prefix */
    void resolveAlert(const std::string& prefix);

    /** @brief Format double value for alert messages */
    static std::string formatValue(double val);

    std::vector<Alert> m_activeAlerts;
    std::vector<Alert> m_alertHistory;

    // Configurable thresholds
    int m_speedLimit;
    int m_engineTempCritical;
    int m_engineTempWarning;
    double m_batteryVoltageMin;
    int m_tirePressureMin;

    mutable std::mutex m_mutex;
    mutable std::mutex m_configMutex;
};

} // namespace VehicleSystem
