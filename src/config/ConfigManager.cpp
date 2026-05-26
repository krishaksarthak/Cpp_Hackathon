#include "config/ConfigManager.hpp"

namespace VehicleSystem {

bool ConfigManager::loadConfig(const std::string& configPath) {
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

bool ConfigManager::reload() {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        path = m_configPath;
    }
    if (path.empty()) return false;
    return loadConfig(path);
}

bool ConfigManager::isLoaded() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_loaded;
}

std::string ConfigManager::getLastError() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastError;
}

int ConfigManager::getInt(const std::string& section, const std::string& key, int defaultVal) const {
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

double ConfigManager::getDouble(const std::string& section, const std::string& key, double defaultVal) const {
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

std::string ConfigManager::getString(const std::string& section, const std::string& key,
                      const std::string& defaultVal) const {
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

bool ConfigManager::getBool(const std::string& section, const std::string& key, bool defaultVal) const {
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

std::string ConfigManager::getActiveProfile() const {
    return getString("profiles", "active_profile", "comfort_mode");
}

void ConfigManager::setActiveProfile(const std::string& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeProfile = profile;
}

std::string ConfigManager::getCurrentProfile() const {
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

std::string ConfigManager::getConfigPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_configPath;
}

} // namespace VehicleSystem
