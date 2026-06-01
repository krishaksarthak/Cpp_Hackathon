#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
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
    bool loadProfile(const std::string& profilePath);

    // --- Getters ---
    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_description; }
    uint32_t getSpeedLimit() const { return m_speedLimit; }
    int32_t getEngineTempWarning() const { return m_engineTempWarning; }
    int32_t getEngineTempCritical() const { return m_engineTempCritical; }
    bool getAggressiveAccelAlert() const { return m_aggressiveAccelAlert; }
    bool getHarshBrakingAlert() const { return m_harshBrakingAlert; }
    std::string getAlertSensitivity() const { return m_alertSensitivity; }
    std::string getLastError() const { return m_lastError; }

    /** @brief Get formatted profile summary */
    std::string getSummary() const;

    /**
     * @brief Get list of available profile files in a directory.
     * This is a static utility method.
     */
    static std::vector<std::string> getAvailableProfiles();

private:
    std::string m_name;
    std::string m_description;
    uint32_t m_speedLimit;
    int32_t m_engineTempWarning;
    int32_t m_engineTempCritical;
    bool m_aggressiveAccelAlert;
    bool m_harshBrakingAlert;
    std::string m_alertSensitivity;
    std::string m_lastError;
};

} // namespace VehicleSystem
