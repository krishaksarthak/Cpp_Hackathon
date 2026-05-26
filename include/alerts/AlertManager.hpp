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
        const std::vector<std::unique_ptr<Sensor>>& sensors) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<Alert> newAlerts;

        double engineTemp = 0, batteryVoltage = 12.6, speed = 0;
        double tirePressure = 32;
        bool doorOpen = false, seatbeltUnlocked = false;

        // Read sensor values via polymorphism
        for (const auto& sensor : sensors) {
            switch (sensor->getType()) {
                case SensorType::ENGINE_TEMPERATURE:
                    engineTemp = sensor->getValue();
                    break;
                case SensorType::BATTERY_VOLTAGE:
                    batteryVoltage = sensor->getValue();
                    break;
                case SensorType::VEHICLE_SPEED:
                    speed = sensor->getValue();
                    break;
                case SensorType::TIRE_PRESSURE:
                    tirePressure = sensor->getValue();
                    break;
                case SensorType::DOOR_STATUS: {
                    auto* doorSensor = dynamic_cast<const DoorSensor*>(sensor.get());
                    doorOpen = doorSensor ? doorSensor->isOpen() : (sensor->getValue() > 0.5);
                    break;
                }
                case SensorType::SEATBELT_STATUS: {
                    auto* seatbeltSensor = dynamic_cast<const SeatbeltSensor*>(sensor.get());
                    seatbeltUnlocked = seatbeltSensor ? !seatbeltSensor->isLocked()
                                                       : (sensor->getValue() < 0.5);
                    break;
                }
            }
        }

        // Condition 1: Engine Overheat
        if (engineTemp > m_engineTempCritical) {
            if (!hasActiveAlert("ENGINE OVERHEAT")) {
                Alert alert(AlertSeverity::CRITICAL,
                           "ENGINE OVERHEAT - Temp: " + formatValue(engineTemp) + "C",
                           SensorType::ENGINE_TEMPERATURE, engineTemp);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("ENGINE OVERHEAT");
        }

        // Condition 2: Low Battery
        if (batteryVoltage < m_batteryVoltageMin) {
            if (!hasActiveAlert("LOW BATTERY")) {
                Alert alert(AlertSeverity::WARNING,
                           "LOW BATTERY - Voltage: " + formatValue(batteryVoltage) + "V",
                           SensorType::BATTERY_VOLTAGE, batteryVoltage);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("LOW BATTERY");
        }

        // Condition 3: Low Tire Pressure
        if (tirePressure < m_tirePressureMin) {
            if (!hasActiveAlert("LOW TIRE PRESSURE")) {
                Alert alert(AlertSeverity::WARNING,
                           "LOW TIRE PRESSURE - Pressure: " + formatValue(tirePressure) + " PSI",
                           SensorType::TIRE_PRESSURE, tirePressure);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("LOW TIRE PRESSURE");
        }

        // Condition 4: Overspeed
        if (speed > m_speedLimit) {
            if (!hasActiveAlert("OVERSPEED")) {
                Alert alert(AlertSeverity::WARNING,
                           "OVERSPEED - Speed: " + formatValue(speed) + " km/h",
                           SensorType::VEHICLE_SPEED, speed);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("OVERSPEED");
        }

        // Condition 5: Door Open While Moving
        if (doorOpen && speed > 10.0) {
            if (!hasActiveAlert("DOOR OPEN WARNING")) {
                Alert alert(AlertSeverity::CRITICAL,
                           "DOOR OPEN WARNING - Speed: " + formatValue(speed) + " km/h",
                           SensorType::DOOR_STATUS, speed);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("DOOR OPEN WARNING");
        }

        // Condition 6: Seatbelt Unlocked While Moving
        if (seatbeltUnlocked && speed > 10.0) {
            if (!hasActiveAlert("SEATBELT WARNING")) {
                Alert alert(AlertSeverity::WARNING,
                           "SEATBELT WARNING - Seatbelt unlocked while moving",
                           SensorType::SEATBELT_STATUS, speed);
                m_activeAlerts.push_back(alert);
                m_alertHistory.push_back(alert);
                newAlerts.push_back(alert);
            }
        } else {
            resolveAlert("SEATBELT WARNING");
        }

        return newAlerts;
    }

    /** @brief Get currently active alerts */
    std::vector<Alert> getActiveAlerts() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeAlerts;
    }

    /** @brief Get full alert history */
    std::vector<Alert> getAlertHistory() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_alertHistory;
    }

    size_t getActiveAlertCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeAlerts.size();
    }

    size_t getTotalAlertCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_alertHistory.size();
    }

    /** @brief Remove all inactive alerts from active list */
    void clearResolvedAlerts() {
        std::lock_guard<std::mutex> lock(m_mutex);
        // STL algorithm: remove_if with lambda
        m_activeAlerts.erase(
            std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
                [](const Alert& a) { return !a.isActive(); }),
            m_activeAlerts.end());
    }

    /**
     * @brief Filter alerts by severity (demonstrates lambda + STL)
     */
    std::vector<Alert> filterAlerts(AlertSeverity severity) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<Alert> filtered;
        // STL algorithm: copy_if with lambda
        std::copy_if(m_alertHistory.begin(), m_alertHistory.end(),
            std::back_inserter(filtered),
            [severity](const Alert& a) { return a.getSeverity() == severity; });
        return filtered;
    }

    // --- Configurable thresholds (for driver profiles) ---
    void setSpeedLimit(int limit) { m_speedLimit = limit; }
    void setEngineTempCritical(int temp) { m_engineTempCritical = temp; }
    void setEngineTempWarning(int temp) { m_engineTempWarning = temp; }
    void setBatteryVoltageMin(double voltage) { m_batteryVoltageMin = voltage; }
    void setTirePressureMin(int pressure) { m_tirePressureMin = pressure; }

    int getSpeedLimit() const { return m_speedLimit; }
    int getEngineTempCritical() const { return m_engineTempCritical; }

private:
    /** @brief Check if an active alert with given message prefix exists */
    bool hasActiveAlert(const std::string& prefix) const {
        // Lambda with std::any_of
        return std::any_of(m_activeAlerts.begin(), m_activeAlerts.end(),
            [&prefix](const Alert& a) {
                return a.isActive() && a.getMessage().find(prefix) != std::string::npos;
            });
    }

    /** @brief Resolve (deactivate) alerts matching prefix */
    void resolveAlert(const std::string& prefix) {
        // Lambda with std::for_each
        std::for_each(m_activeAlerts.begin(), m_activeAlerts.end(),
            [&prefix](Alert& a) {
                if (a.isActive() && a.getMessage().find(prefix) != std::string::npos) {
                    a.deactivate();
                }
            });
        // Clean up inactive alerts
        m_activeAlerts.erase(
            std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
                [](const Alert& a) { return !a.isActive(); }),
            m_activeAlerts.end());
    }

    /** @brief Format double value for alert messages */
    static std::string formatValue(double val) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << val;
        return oss.str();
    }

    std::vector<Alert> m_activeAlerts;
    std::vector<Alert> m_alertHistory;

    // Configurable thresholds
    int m_speedLimit;
    int m_engineTempCritical;
    int m_engineTempWarning;
    double m_batteryVoltageMin;
    int m_tirePressureMin;

    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
