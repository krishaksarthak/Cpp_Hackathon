#pragma once

#include <string>
#include <mutex>
#include "common/JsonParser.hpp"
#include "common/Types.hpp"
#include <cstdint>

namespace VehicleSystem {

/**
 * @brief Manages runtime configuration loaded from JSON.
 * 
 * Demonstrates: Singleton pattern, file I/O, exception handling,
 * thread-safe access, runtime reconfiguration.
 * 
 * BONUS FEATURE: JSON Configuration Management
 * 
 * @author Member 5 (Integration Lead)
 */
class ConfigManager {
public:
    /** @brief Get singleton instance */
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // Delete copy and move (singleton)
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    /**
     * @brief Load configuration from JSON file.
     * @param configPath Path to the config.json file
     * @return true if loaded successfully
     */
    bool loadConfig(const std::string& configPath);

    /**
     * @brief Reload configuration from the same file path.
     * @return true if reloaded successfully
     */
    bool reload();

    /** @brief Check if config is loaded */
    bool isLoaded() const;

    /** @brief Get last error message */
    std::string getLastError() const;

    // --- Typed getters with section/key access ---

    int32_t getInt(const std::string& section, const std::string& key, int32_t defaultVal = 0) const;

    uint32_t getUint32(const std::string& section, const std::string& key, uint32_t defaultVal = 0) const;

    double getDouble(const std::string& section, const std::string& key, double defaultVal = 0.0) const;

    std::string getString(const std::string& section, const std::string& key,
                          const std::string& defaultVal = "") const;

    bool getBool(const std::string& section, const std::string& key, bool defaultVal = false) const;

    /** @brief Get active profile name */
    std::string getActiveProfile() const;

    /** @brief Set active profile name */
    void setActiveProfile(const std::string& profile);

    /** @brief Get active profile (override or from config) */
    std::string getCurrentProfile() const;

    /** @brief Get the config file path */
    std::string getConfigPath() const;

private:
    ConfigManager() : m_loaded(false) {}

    std::string m_configPath;
    JsonValue m_configData;
    bool m_loaded;
    std::string m_lastError;
    std::string m_activeProfile;
    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
