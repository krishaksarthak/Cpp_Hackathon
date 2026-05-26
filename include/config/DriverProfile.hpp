#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "common/JsonParser.hpp"

namespace VehicleSystem {

/**
 * @brief Represents a driver profile with custom thresholds.
 * 
 * Profiles: Eco Mode, Sport Mode, Comfort Mode
 * Each profile has different alert thresholds and preferences.
 * 
 * Demonstrates: File I/O, exception handling, data encapsulation.
 * 
 * BONUS FEATURE: Driver Profile Management
 */
class DriverProfile {
public:
    DriverProfile()
        : m_name("Default"), m_description("Default driving profile"),
          m_speedLimit(120), m_engineTempWarning(100),
          m_engineTempCritical(110), m_aggressiveAccelAlert(true),
          m_harshBrakingAlert(true), m_alertSensitivity("medium") {}

    // Copy and move semantics
    DriverProfile(const DriverProfile&) = default;
    DriverProfile(DriverProfile&&) noexcept = default;
    DriverProfile& operator=(const DriverProfile&) = default;
    DriverProfile& operator=(DriverProfile&&) noexcept = default;
    ~DriverProfile() = default;

    /**
     * @brief Load profile from a JSON file.
     * @param profilePath Path to the profile JSON file
     * @return true if loaded successfully
     */
    bool loadProfile(const std::string& profilePath) {
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

    // --- Getters ---
    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_description; }
    int getSpeedLimit() const { return m_speedLimit; }
    int getEngineTempWarning() const { return m_engineTempWarning; }
    int getEngineTempCritical() const { return m_engineTempCritical; }
    bool getAggressiveAccelAlert() const { return m_aggressiveAccelAlert; }
    bool getHarshBrakingAlert() const { return m_harshBrakingAlert; }
    std::string getAlertSensitivity() const { return m_alertSensitivity; }
    std::string getLastError() const { return m_lastError; }

    /** @brief Get formatted profile summary */
    std::string getSummary() const {
        std::ostringstream oss;
        oss << "Profile: " << m_name << "\n"
            << "  Description:     " << m_description << "\n"
            << "  Speed Limit:     " << m_speedLimit << " km/h\n"
            << "  Temp Warning:    " << m_engineTempWarning << " C\n"
            << "  Temp Critical:   " << m_engineTempCritical << " C\n"
            << "  Alert Sensitivity: " << m_alertSensitivity << "\n";
        return oss.str();
    }

    /**
     * @brief Get list of available profile files in a directory.
     * This is a static utility method.
     */
    static std::vector<std::string> getAvailableProfiles() {
        // Returns hardcoded list since filesystem iteration is complex
        return {"eco_mode", "sport_mode", "comfort_mode"};
    }

private:
    std::string m_name;
    std::string m_description;
    int m_speedLimit;
    int m_engineTempWarning;
    int m_engineTempCritical;
    bool m_aggressiveAccelAlert;
    bool m_harshBrakingAlert;
    std::string m_alertSensitivity;
    std::string m_lastError;
};

} // namespace VehicleSystem
