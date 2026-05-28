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

    std::string asil;         // e.g., "C" — ISO 26262 ASIL level
    std::string hazard;       // Hazard description from HARA
    std::string safetyGoal;   // Safety goal from HARA

    /** @brief Format DTC for display including ISO 26262 ASIL level */
    std::string format() const {
        std::ostringstream oss;
        oss << code << " - " << description
            << " [" << severityToString(severity) << "]"
            << " [" << asil << "]"
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
    DTCManager();

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
                     const std::string& activeProfile);

    /** @brief Get all active DTCs */
    std::vector<DiagnosticTroubleCode> getActiveDTCs() const;

    /** @brief Get full DTC history */
    std::vector<DiagnosticTroubleCode> getDTCHistory() const;

    /** @brief Clear a specific DTC by code */
    void clearDTC(const std::string& code);

    /** @brief Clear all active DTCs */
    void clearAllDTCs();

    /** @brief Get count of active DTCs */
    size_t getActiveDTCCount() const;

    /** @brief Get formatted DTC list for display */
    std::string getFormattedDTCList() const;

private:
    /** @brief Initialize the mapping from alert types to DTC codes */
    void initializeDTCMappings();

    /** @brief Map an alert message to a DTC code */
    std::string mapAlertToDTC(const Alert& alert) const;

    std::string getDTCDescription(const std::string& code) const;

    std::string getDTCCategory(const std::string& code) const;

    std::vector<DiagnosticTroubleCode> m_activeDTCs;
    std::vector<DiagnosticTroubleCode> m_dtcHistory;
    std::map<std::string, std::string> m_alertToDTCMap;
    std::map<std::string, std::string> m_dtcDescriptions;
    std::map<std::string, std::string> m_dtcCategories;
    std::map<std::string, std::string> m_dtcASIL;         ///< ISO 26262 ASIL per DTC
    std::map<std::string, std::string> m_dtcHazard;       ///< Hazard description per DTC
    std::map<std::string, std::string> m_dtcSafetyGoal;   ///< Safety goal per DTC

    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
