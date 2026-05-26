#include "alerts/DTC.hpp"

namespace VehicleSystem {

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

std::vector<DiagnosticTroubleCode> DTCManager::getActiveDTCs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeDTCs;
}

std::vector<DiagnosticTroubleCode> DTCManager::getDTCHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_dtcHistory;
}

void DTCManager::clearDTC(const std::string& code) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeDTCs.erase(
        std::remove_if(m_activeDTCs.begin(), m_activeDTCs.end(),
            [&code](const DiagnosticTroubleCode& dtc) { return dtc.code == code; }),
        m_activeDTCs.end());
}

void DTCManager::clearAllDTCs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Mark all as inactive in history before clearing
    for (auto& dtc : m_activeDTCs) {
        dtc.active = false;
    }
    m_activeDTCs.clear();
}

size_t DTCManager::getActiveDTCCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeDTCs.size();
}

std::string DTCManager::getFormattedDTCList() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_activeDTCs.empty()) return "No active DTCs";
    std::ostringstream oss;
    for (const auto& dtc : m_activeDTCs) {
        oss << dtc.format() << "\n";
    }
    return oss.str();
}

void DTCManager::initializeDTCMappings() {
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

std::string DTCManager::mapAlertToDTC(const Alert& alert) const {
    for (const auto& pair : m_alertToDTCMap) {
        if (alert.getMessage().find(pair.first) != std::string::npos) {
            return pair.second;
        }
    }
    return "";
}

std::string DTCManager::getDTCDescription(const std::string& code) const {
    auto it = m_dtcDescriptions.find(code);
    return it != m_dtcDescriptions.end() ? it->second : "Unknown DTC";
}

std::string DTCManager::getDTCCategory(const std::string& code) const {
    auto it = m_dtcCategories.find(code);
    return it != m_dtcCategories.end() ? it->second : "Unknown";
}

} // namespace VehicleSystem
