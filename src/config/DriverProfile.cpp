#include "config/DriverProfile.hpp"
#include <sstream>

namespace VehicleSystem {

bool DriverProfile::loadProfile(const std::string& profilePath) {
    try {
        JsonValue data = JsonValue::parseFile(profilePath);

        if (data.hasKey("profile_name"))
            m_name = data["profile_name"].getString();
        if (data.hasKey("description"))
            m_description = data["description"].getString();
        if (data.hasKey("alert_sensitivity"))
            m_alertSensitivity = data["alert_sensitivity"].getString();

        if (data.hasKey("thresholds")) {
            const auto& thresholds = data["thresholds"];
            if (thresholds.hasKey("speed_limit"))
                m_speedLimit = thresholds["speed_limit"].getInt();
            if (thresholds.hasKey("engine_temp_warning"))
                m_engineTempWarning = thresholds["engine_temp_warning"].getInt();
            if (thresholds.hasKey("engine_temp_critical"))
                m_engineTempCritical = thresholds["engine_temp_critical"].getInt();
            if (thresholds.hasKey("aggressive_acceleration_alert"))
                m_aggressiveAccelAlert = thresholds["aggressive_acceleration_alert"].getBool();
            if (thresholds.hasKey("harsh_braking_alert"))
                m_harshBrakingAlert = thresholds["harsh_braking_alert"].getBool();
        }

        return true;
    } catch (const std::exception& e) {
        m_lastError = e.what();
        return false;
    }
}

std::string DriverProfile::getSummary() const {
    std::ostringstream oss;
    oss << "Profile: " << m_name << "\n"
        << "  Description:     " << m_description << "\n"
        << "  Speed Limit:     " << m_speedLimit << " km/h\n"
        << "  Temp Warning:    " << m_engineTempWarning << " C\n"
        << "  Temp Critical:   " << m_engineTempCritical << " C\n"
        << "  Alert Sensitivity: " << m_alertSensitivity << "\n";
    return oss.str();
}

std::vector<std::string> DriverProfile::getAvailableProfiles() {
    // Returns hardcoded list since filesystem iteration is complex
    return {"eco_mode", "sport_mode", "comfort_mode"};
}

} // namespace VehicleSystem
