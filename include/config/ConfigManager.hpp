#pragma once

#include <string>
#include <mutex>
#include "common/JsonParser.hpp"

namespace VehicleSystem {

/**
 * @brief Manages runtime configuration loaded from JSON.
 * 
 * Demonstrates: Singleton pattern, file I/O, exception handling,
 * thread-safe access, runtime reconfiguration.
 * 
 * BONUS FEATURE: JSON Configuration Management
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
    bool loadConfig(const std::string& configPath) {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            m_configData = JsonValue::parseFile(configPath);
            m_configPath = configPath;
            m_loaded = true;
            return true;
        } catch (const std::exception& e) {
            m_lastError = e.what();
            m_loaded = false;
            return false;
        }
    }

    /**
     * @brief Reload configuration from the same file path.
     * @return true if reloaded successfully
     */
    bool reload() {
        std::string path;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            path = m_configPath;
        }
        if (path.empty()) return false;
        return loadConfig(path);
    }

    /** @brief Check if config is loaded */
    bool isLoaded() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_loaded;
    }

    /** @brief Get last error message */
    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lastError;
    }

    // --- Typed getters with section/key access ---

    int getInt(const std::string& section, const std::string& key, int defaultVal = 0) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            if (m_loaded && m_configData.hasKey(section)) {
                const auto& sec = m_configData[section];
                if (sec.hasKey(key)) {
                    return sec[key].getInt();
                }
            }
        } catch (...) {}
        return defaultVal;
    }

    double getDouble(const std::string& section, const std::string& key, double defaultVal = 0.0) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            if (m_loaded && m_configData.hasKey(section)) {
                const auto& sec = m_configData[section];
                if (sec.hasKey(key)) {
                    return sec[key].getDouble();
                }
            }
        } catch (...) {}
        return defaultVal;
    }

    std::string getString(const std::string& section, const std::string& key,
                          const std::string& defaultVal = "") const {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            if (m_loaded && m_configData.hasKey(section)) {
                const auto& sec = m_configData[section];
                if (sec.hasKey(key)) {
                    return sec[key].getString();
                }
            }
        } catch (...) {}
        return defaultVal;
    }

    bool getBool(const std::string& section, const std::string& key, bool defaultVal = false) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            if (m_loaded && m_configData.hasKey(section)) {
                const auto& sec = m_configData[section];
                if (sec.hasKey(key)) {
                    return sec[key].getBool();
                }
            }
        } catch (...) {}
        return defaultVal;
    }

    /** @brief Get active profile name */
    std::string getActiveProfile() const {
        return getString("profiles", "active_profile", "comfort_mode");
    }

    /** @brief Set active profile name */
    void setActiveProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeProfile = profile;
    }

    /** @brief Get active profile (override or from config) */
    std::string getCurrentProfile() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_activeProfile.empty()) return m_activeProfile;
        // Read directly from config data under the same lock
        try {
            if (m_loaded && m_configData.hasKey("profiles")) {
                const auto& sec = m_configData["profiles"];
                if (sec.hasKey("active_profile")) {
                    return sec["active_profile"].getString();
                }
            }
        } catch (...) {}
        return "comfort_mode";
    }

    /** @brief Get the config file path */
    std::string getConfigPath() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_configPath;
    }

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
