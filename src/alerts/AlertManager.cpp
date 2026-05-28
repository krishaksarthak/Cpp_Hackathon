#include "alerts/AlertManager.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace VehicleSystem {

// This function provides the implementation for evaluateConditions
std::vector<Alert> AlertManager::evaluateConditions(
    const std::vector<std::unique_ptr<Sensor>>& sensors) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Alert> newAlerts;

    // Thread-safe copy of thresholds
    int currentEngineTempCritical, currentBatteryVoltageMin;
    int currentTirePressureMin, currentSpeedLimit;
    {
        std::lock_guard<std::mutex> configLock(m_configMutex);
        currentEngineTempCritical = m_engineTempCritical;
        currentBatteryVoltageMin = m_batteryVoltageMin;
        currentTirePressureMin = m_tirePressureMin;
        currentSpeedLimit = m_speedLimit;
    }

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

    // Condition 1: Engine Overheat [ISO 26262 ASIL C]
    // Safety Goal SG-01: Detect within 500ms, prevent thermal runaway
    if (engineTemp > currentEngineTempCritical) {
        if (!hasActiveAlert("ENGINE OVERHEAT")) {
            Alert alert(AlertSeverity::CRITICAL,
                       "ENGINE OVERHEAT - Temp: " + formatValue(engineTemp) + "C",
                       SensorType::ENGINE_TEMPERATURE, engineTemp,
                       ASILLevel::C);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("ENGINE OVERHEAT");
    }

    // Condition 2: Low Battery [ISO 26262 ASIL B]
    // Safety Goal SG-02: Warn before complete power loss
    if (batteryVoltage < currentBatteryVoltageMin) {
        if (!hasActiveAlert("LOW BATTERY")) {
            Alert alert(AlertSeverity::WARNING,
                       "LOW BATTERY - Voltage: " + formatValue(batteryVoltage) + "V",
                       SensorType::BATTERY_VOLTAGE, batteryVoltage,
                       ASILLevel::B);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("LOW BATTERY");
    }

    // Condition 3: Low Tire Pressure [ISO 26262 ASIL B]
    // Safety Goal SG-04: Detect blowout risk in real time
    if (tirePressure < currentTirePressureMin) {
        if (!hasActiveAlert("LOW TIRE PRESSURE")) {
            Alert alert(AlertSeverity::WARNING,
                       "LOW TIRE PRESSURE - Pressure: " + formatValue(tirePressure) + " PSI",
                       SensorType::TIRE_PRESSURE, tirePressure,
                       ASILLevel::B);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("LOW TIRE PRESSURE");
    }

    // Condition 4: Overspeed [ISO 26262 ASIL B]
    // Safety Goal SG-03: Alert driver when speed exceeds profile limit
    if (speed > currentSpeedLimit) {
        if (!hasActiveAlert("OVERSPEED")) {
            Alert alert(AlertSeverity::WARNING,
                       "OVERSPEED - Speed: " + formatValue(speed) + " km/h",
                       SensorType::VEHICLE_SPEED, speed,
                       ASILLevel::B);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("OVERSPEED");
    }

    // Condition 5: Door Open While Moving [ISO 26262 ASIL C]
    // Safety Goal SG-05: Immediate CRITICAL alert above 10 km/h
    if (doorOpen && speed > 10.0) {
        if (!hasActiveAlert("DOOR OPEN WARNING")) {
            Alert alert(AlertSeverity::CRITICAL,
                       "DOOR OPEN WARNING - Speed: " + formatValue(speed) + " km/h",
                       SensorType::DOOR_STATUS, speed,
                       ASILLevel::C);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("DOOR OPEN WARNING");
    }

    // Condition 6: Seatbelt Unlocked While Moving [ISO 26262 ASIL B]
    // Safety Goal SG-06: Continuously alert while moving without seatbelt
    if (seatbeltUnlocked && speed > 10.0) {
        if (!hasActiveAlert("SEATBELT WARNING")) {
            Alert alert(AlertSeverity::WARNING,
                       "SEATBELT WARNING - Seatbelt unlocked while moving",
                       SensorType::SEATBELT_STATUS, speed,
                       ASILLevel::B);
            m_activeAlerts.push_back(alert);
            m_alertHistory.push_back(alert);
            newAlerts.push_back(alert);
        }
    } else {
        resolveAlert("SEATBELT WARNING");
    }

    return newAlerts;
}

// This function provides the implementation for getActiveAlerts
std::vector<Alert> AlertManager::getActiveAlerts() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeAlerts;
}

// This function provides the implementation for getAlertHistory
std::vector<Alert> AlertManager::getAlertHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alertHistory;
}

// This function provides the implementation for getActiveAlertCount
size_t AlertManager::getActiveAlertCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeAlerts.size();
}

// This function provides the implementation for getTotalAlertCount
size_t AlertManager::getTotalAlertCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alertHistory.size();
}
// This function provides the implementation for destructor

// This function provides the implementation for clearResolvedAlerts
void AlertManager::clearResolvedAlerts() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // STL algorithm: remove_if with lambda
    m_activeAlerts.erase(
        std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
            [](const Alert& a) { return !a.isActive(); }),
        m_activeAlerts.end());
}

std::vector<Alert> AlertManager::filterAlerts(AlertSeverity severity) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Alert> filtered;
    // STL algorithm: copy_if with lambda
    std::copy_if(m_alertHistory.begin(), m_alertHistory.end(),
        std::back_inserter(filtered),
        [severity](const Alert& a) { return a.getSeverity() == severity; });
    return filtered;
}

// This function provides the implementation for hasActiveAlert
bool AlertManager::hasActiveAlert(const std::string& prefix) const {
    // Lambda with std::any_of
    return std::any_of(m_activeAlerts.begin(), m_activeAlerts.end(),
        [&prefix](const Alert& a) {
            return a.isActive() && a.getMessage().find(prefix) != std::string::npos;
        });
}

// This function provides the implementation for resolveAlert
void AlertManager::resolveAlert(const std::string& prefix) {
    // Lambda with std::for_each
    std::for_each(m_activeAlerts.begin(), m_activeAlerts.end(),
// This function provides the implementation for destructor
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

// This function provides the implementation for formatValue
std::string AlertManager::formatValue(double val) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << val;
    return oss.str();
}

} // namespace VehicleSystem
