#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <algorithm>
#include "common/Types.hpp"
#include "common/Utils.hpp"
#include "alerts/Alert.hpp"
#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Freeze frame data captured at the time of DTC generation.
 * Snapshot of all sensor values when a fault occurred.
 */
struct FreezeFrame {
    Timestamp timestamp;
    std::map<SensorType, double> sensorValues;
    std::string activeProfile;
    double vehicleSpeed;
    double engineTemp;
    double batteryVoltage;

    FreezeFrame()
        : timestamp(std::chrono::system_clock::now()),
          vehicleSpeed(0), engineTemp(0), batteryVoltage(0) {}
};

/**
 * @brief Represents a Diagnostic Trouble Code (automotive standard).
 * 
 * Standard format: Pxxxx (Powertrain), Cxxxx (Chassis), 
 * Bxxxx (Body), Uxxxx (Network)
 */
struct DiagnosticTroubleCode {
    std::string code;           // e.g., "P0217"
    std::string description;    // e.g., "Engine Coolant Over Temperature Condition"
    AlertSeverity severity;
    std::string category;       // e.g., "Powertrain"
    FreezeFrame freezeFrame;
    Timestamp firstOccurrence;
    Timestamp lastOccurrence;
    int occurrenceCount;
    bool active;

    DiagnosticTroubleCode()
        : severity(AlertSeverity::INFO),
          firstOccurrence(std::chrono::system_clock::now()),
          lastOccurrence(std::chrono::system_clock::now()),
          occurrenceCount(1), active(true) {}

    /** @brief Format DTC for display */
    std::string format() const {
        std::ostringstream oss;
        oss << code << " - " << description
            << " [" << severityToString(severity) << "]"
            << " (" << occurrenceCount << "x)";
        return oss.str();
    }
};

/**
 * @brief Manages Diagnostic Trouble Codes (DTCs).
 * 
 * Demonstrates: STL containers (map, vector), exception handling,
 * complex data structures, industry-standard patterns.
 * Maps to Diagnostic Event Manager (DEM) in AUTOSAR.
 * 
 * BONUS FEATURE: DTC System with freeze-frame data
 */
class DTCManager {
public:
    DTCManager() {
        initializeDTCMappings();
    }

    ~DTCManager() = default;

    // Delete copy
    DTCManager(const DTCManager&) = delete;
    DTCManager& operator=(const DTCManager&) = delete;

    /**
     * @brief Generate a DTC from an alert, capturing freeze frame data.
     * 
     * @param alert The alert that triggered the DTC
     * @param sensors Current sensor readings for freeze frame
     * @param activeProfile Current driving profile name
     */
    void generateDTC(const Alert& alert,
                     const std::vector<std::unique_ptr<Sensor>>& sensors,
                     const std::string& activeProfile) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string dtcCode = mapAlertToDTC(alert);
        if (dtcCode.empty()) return;

        // Check if DTC already exists (increment count)
        auto it = std::find_if(m_activeDTCs.begin(), m_activeDTCs.end(),
            [&dtcCode](const DiagnosticTroubleCode& dtc) {
                return dtc.code == dtcCode && dtc.active;
            });

        if (it != m_activeDTCs.end()) {
            it->occurrenceCount++;
            it->lastOccurrence = std::chrono::system_clock::now();
            return;
        }

        // Create new DTC with freeze frame
        DiagnosticTroubleCode dtc;
        dtc.code = dtcCode;
        dtc.description = getDTCDescription(dtcCode);
        dtc.severity = alert.getSeverity();
        dtc.category = getDTCCategory(dtcCode);
        dtc.active = true;

        // Capture freeze frame (snapshot of all sensors)
        dtc.freezeFrame.timestamp = std::chrono::system_clock::now();
        dtc.freezeFrame.activeProfile = activeProfile;
        for (const auto& sensor : sensors) {
            dtc.freezeFrame.sensorValues[sensor->getType()] = sensor->getValue();
            if (sensor->getType() == SensorType::VEHICLE_SPEED)
                dtc.freezeFrame.vehicleSpeed = sensor->getValue();
            if (sensor->getType() == SensorType::ENGINE_TEMPERATURE)
                dtc.freezeFrame.engineTemp = sensor->getValue();
            if (sensor->getType() == SensorType::BATTERY_VOLTAGE)
                dtc.freezeFrame.batteryVoltage = sensor->getValue();
        }

        m_activeDTCs.push_back(dtc);
        m_dtcHistory.push_back(dtc);
    }

    /** @brief Get all active DTCs */
    std::vector<DiagnosticTroubleCode> getActiveDTCs() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeDTCs;
    }

    /** @brief Get full DTC history */
    std::vector<DiagnosticTroubleCode> getDTCHistory() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_dtcHistory;
    }

    /** @brief Clear a specific DTC by code */
    void clearDTC(const std::string& code) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeDTCs.erase(
            std::remove_if(m_activeDTCs.begin(), m_activeDTCs.end(),
                [&code](const DiagnosticTroubleCode& dtc) { return dtc.code == code; }),
            m_activeDTCs.end());
    }

    /** @brief Clear all active DTCs */
    void clearAllDTCs() {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Mark all as inactive in history before clearing
        for (auto& dtc : m_activeDTCs) {
            dtc.active = false;
        }
        m_activeDTCs.clear();
    }

    /** @brief Get count of active DTCs */
    size_t getActiveDTCCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeDTCs.size();
    }

    /** @brief Get formatted DTC list for display */
    std::string getFormattedDTCList() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_activeDTCs.empty()) return "No active DTCs";
        std::ostringstream oss;
        for (const auto& dtc : m_activeDTCs) {
            oss << dtc.format() << "\n";
        }
        return oss.str();
    }

private:
    /** @brief Initialize the mapping from alert types to DTC codes */
    void initializeDTCMappings() {
        m_alertToDTCMap["ENGINE OVERHEAT"] = "P0217";
        m_alertToDTCMap["LOW BATTERY"] = "P0562";
        m_alertToDTCMap["LOW TIRE PRESSURE"] = "C0035";
        m_alertToDTCMap["OVERSPEED"] = "P0128";
        m_alertToDTCMap["DOOR OPEN WARNING"] = "B0001";
        m_alertToDTCMap["SEATBELT WARNING"] = "U0001";

        m_dtcDescriptions["P0217"] = "Engine Coolant Over Temperature Condition";
        m_dtcDescriptions["P0562"] = "System Voltage Low";
        m_dtcDescriptions["P0128"] = "Coolant Thermostat Temperature Below Regulating Temperature";
        m_dtcDescriptions["C0035"] = "Left Front Wheel Speed Sensor Circuit";
        m_dtcDescriptions["B0001"] = "Driver Airbag Circuit Short to Ground";
        m_dtcDescriptions["U0001"] = "High Speed CAN Communication Bus";

        m_dtcCategories["P0217"] = "Powertrain";
        m_dtcCategories["P0562"] = "Electrical";
        m_dtcCategories["P0128"] = "Powertrain";
        m_dtcCategories["C0035"] = "Chassis";
        m_dtcCategories["B0001"] = "Body";
        m_dtcCategories["U0001"] = "Network";
    }

    /** @brief Map an alert message to a DTC code */
    std::string mapAlertToDTC(const Alert& alert) const {
        for (const auto& [prefix, code] : m_alertToDTCMap) {
            if (alert.getMessage().find(prefix) != std::string::npos) {
                return code;
            }
        }
        return "";
    }

    std::string getDTCDescription(const std::string& code) const {
        auto it = m_dtcDescriptions.find(code);
        return it != m_dtcDescriptions.end() ? it->second : "Unknown DTC";
    }

    std::string getDTCCategory(const std::string& code) const {
        auto it = m_dtcCategories.find(code);
        return it != m_dtcCategories.end() ? it->second : "Unknown";
    }

    std::vector<DiagnosticTroubleCode> m_activeDTCs;
    std::vector<DiagnosticTroubleCode> m_dtcHistory;
    std::map<std::string, std::string> m_alertToDTCMap;
    std::map<std::string, std::string> m_dtcDescriptions;
    std::map<std::string, std::string> m_dtcCategories;

    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
