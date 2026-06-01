#include "threading/Watchdog.hpp"

namespace VehicleSystem {

Watchdog::Watchdog(uint32_t timeoutMs)
    : m_timeoutMs(timeoutMs), m_startTime(std::chrono::steady_clock::now()) {}

void Watchdog::registerThread(const std::string& threadName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lastHeartbeat[threadName] = std::chrono::steady_clock::now();
    m_threadHealth[threadName] = true;
    m_heartbeatCount[threadName] = 0;
}

void Watchdog::heartbeat(const std::string& threadName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lastHeartbeat[threadName] = std::chrono::steady_clock::now();
    m_threadHealth[threadName] = true;
    m_heartbeatCount[threadName]++;
}

bool Watchdog::checkHealth() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    bool allHealthy = true;

    for (auto& pair : m_lastHeartbeat) {
        const std::string& name = pair.first;
        auto& lastBeat = pair.second;
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastBeat).count();
            
        if (elapsed > m_timeoutMs) {
            if (m_threadHealth[name]) {
                m_threadHealth[name] = false;
                allHealthy = false;
            }
        } else {
            m_threadHealth[name] = true;
        }
    }
    return allHealthy;
}

std::vector<std::string> Watchdog::getUnhealthyThreads() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> unhealthy;
    for (const auto& pair : m_threadHealth) {
        if (!pair.second) {
            unhealthy.push_back(pair.first);
        }
    }
    return unhealthy;
}

std::string Watchdog::getHealthReport() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream oss;
    auto now = std::chrono::steady_clock::now();

    oss << "THREAD HEALTH MONITOR\n";
    oss << std::string(40, '-') << "\n";

    for (const auto& pair : m_threadHealth) {
        const std::string& name = pair.first;
        bool healthy = pair.second;
        
        auto it = m_lastHeartbeat.find(name);
        long long elapsed = 0;
        if (it != m_lastHeartbeat.end()) {
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - it->second).count();
        }

        auto countIt = m_heartbeatCount.find(name);
        uint64_t count = countIt != m_heartbeatCount.end() ? countIt->second : 0;

        oss << std::setw(20) << std::left << name << " | "
            << (healthy ? "HEALTHY" : "TIMEOUT") << " | "
            << elapsed << "ms ago | "
            << count << " beats\n";
    }
    return oss.str();
}

bool Watchdog::isThreadHealthy(const std::string& threadName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_threadHealth.find(threadName);
    return it != m_threadHealth.end() && it->second;
}

size_t Watchdog::getRegisteredThreadCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_threadHealth.size();
}

std::string Watchdog::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
    uint32_t hours = static_cast<uint32_t>(elapsed / 3600);
    uint32_t minutes = static_cast<uint32_t>((elapsed % 3600) / 60);
    uint32_t seconds = static_cast<uint32_t>(elapsed % 60);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds;
    return oss.str();
}

} // namespace VehicleSystem
