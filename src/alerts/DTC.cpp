#include "alerts/DTC.hpp"
#include "common/JsonParser.hpp"
#include <iostream>

namespace VehicleSystem {

// This function provides the implementation for constructor
DTCManager::DTCManager() {
    initializeDTCMappings();
}

void DTCManager::generateDTC(const Alert& alert,
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

    // Populate ISO 26262 safety metadata
    auto asilIt = m_dtcASIL.find(dtcCode);
    dtc.asil = (asilIt != m_dtcASIL.end()) ? ("ASIL " + asilIt->second) : "QM";
    auto hazardIt = m_dtcHazard.find(dtcCode);
    dtc.hazard = (hazardIt != m_dtcHazard.end()) ? hazardIt->second : "";
    auto sgIt = m_dtcSafetyGoal.find(dtcCode);
    dtc.safetyGoal = (sgIt != m_dtcSafetyGoal.end()) ? sgIt->second : "";

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

// This function provides the implementation for getActiveDTCs
std::vector<DiagnosticTroubleCode> DTCManager::getActiveDTCs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeDTCs;
}

// This function provides the implementation for getDTCHistory
std::vector<DiagnosticTroubleCode> DTCManager::getDTCHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_dtcHistory;
}

// This function provides the implementation for clearDTC
void DTCManager::clearDTC(const std::string& code) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeDTCs.erase(
        std::remove_if(m_activeDTCs.begin(), m_activeDTCs.end(),
            [&code](const DiagnosticTroubleCode& dtc) { return dtc.code == code; }),
        m_activeDTCs.end());
}

// This function provides the implementation for clearAllDTCs
void DTCManager::clearAllDTCs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Mark all as inactive in history before clearing
    for (auto& dtc : m_activeDTCs) {
        dtc.active = false;
    }
    m_activeDTCs.clear();
}

// This function provides the implementation for getActiveDTCCount
size_t DTCManager::getActiveDTCCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeDTCs.size();
}

// This function provides the implementation for getFormattedDTCList
std::string DTCManager::getFormattedDTCList() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_activeDTCs.empty()) return "No active DTCs";
    std::ostringstream oss;
    for (const auto& dtc : m_activeDTCs) {
        oss << dtc.format() << "\n";
    }
    return oss.str();
}

// This function provides the implementation for initializeDTCMappings
void DTCManager::initializeDTCMappings() {
    std::string paths[] = { "data/dtc_codes.json", "../data/dtc_codes.json", "../../data/dtc_codes.json" };
    bool loaded = false;

    for (const auto& path : paths) {
        try {
            auto json = JsonValue::parseFile(path);
            if (json.hasKey("dtc_definitions")) {
                const auto& dtcs = json["dtc_definitions"].getObject();
                for (const auto& pair : dtcs) {
                    std::string code = pair.first;
                    const auto& data = pair.second;

                    m_dtcDescriptions[code] = data["description"].get("Unknown");
                    m_dtcCategories[code]   = data["category"].get("Unknown");

                    if (data.hasKey("trigger")) {
                        m_alertToDTCMap[data["trigger"].get("")] = code;
                    }
                    // ISO 26262 fields
                    if (data.hasKey("asil")) {
                        m_dtcASIL[code] = data["asil"].get("QM");
                    }
                    if (data.hasKey("hazard")) {
                        m_dtcHazard[code] = data["hazard"].get("");
                    }
                    if (data.hasKey("safety_goal")) {
                        m_dtcSafetyGoal[code] = data["safety_goal"].get("");
                    }
                }
                loaded = true;
                break;
            }
        } catch (...) {
            // Ignore and try next path
        }
    }

    if (!loaded) {
        std::cerr << "[WARN] Could not load dtc_codes.json. DTCs will not be generated.\n";
    }
}

// This function provides the implementation for mapAlertToDTC
std::string DTCManager::mapAlertToDTC(const Alert& alert) const {
// This function provides the implementation for destructor
    for (const auto& pair : m_alertToDTCMap) {
        if (alert.getMessage().find(pair.first) != std::string::npos) {
            return pair.second;
        }
    }
    return "";
}

// This function provides the implementation for getDTCDescription
std::string DTCManager::getDTCDescription(const std::string& code) const {
    auto it = m_dtcDescriptions.find(code);
    return it != m_dtcDescriptions.end() ? it->second : "Unknown DTC";
}

// This function provides the implementation for getDTCCategory
std::string DTCManager::getDTCCategory(const std::string& code) const {
    auto it = m_dtcCategories.find(code);
    return it != m_dtcCategories.end() ? it->second : "Unknown";
}

} // namespace VehicleSystem
