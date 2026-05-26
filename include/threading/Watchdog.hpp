#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace VehicleSystem {

/**
 * @brief Watchdog timer / health monitor for thread supervision.
 * 
 * Demonstrates: STL containers (map), chrono, mutex, RAII,
 * safety-critical system design patterns.
 * Maps to Watchdog Manager in AUTOSAR OS.
 * 
 * BONUS FEATURE: Thread health monitoring
 */
class Watchdog {
public:
    /**
     * @brief Construct watchdog with specified timeout
     * @param timeoutMs Maximum time (ms) between heartbeats before declaring unhealthy
     */
    explicit Watchdog(int timeoutMs = 5000)
        : m_timeoutMs(timeoutMs), m_startTime(std::chrono::steady_clock::now()) {}

    ~Watchdog() = default;

    // Delete copy (singleton-like usage)
    Watchdog(const Watchdog&) = delete;
    Watchdog& operator=(const Watchdog&) = delete;

    // Allow move
    Watchdog(Watchdog&&) = default;
    Watchdog& operator=(Watchdog&&) = default;

    /**
     * @brief Register a thread for monitoring
     * @param threadName Unique name for the thread
     */
    void registerThread(const std::string& threadName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastHeartbeat[threadName] = std::chrono::steady_clock::now();
        m_threadHealth[threadName] = true;
        m_heartbeatCount[threadName] = 0;
    }

    /**
     * @brief Record a heartbeat from a thread (must be called periodically)
     * @param threadName Name of the thread sending heartbeat
     */
    void heartbeat(const std::string& threadName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastHeartbeat[threadName] = std::chrono::steady_clock::now();
        m_threadHealth[threadName] = true;
        m_heartbeatCount[threadName]++;
    }

    /**
     * @brief Check health of all registered threads
     * @return true if all threads are healthy
     */
    bool checkHealth() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        bool allHealthy = true;

        for (auto& [name, lastBeat] : m_lastHeartbeat) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastBeat).count();
            if (elapsed > m_timeoutMs) {
                m_threadHealth[name] = false;
                allHealthy = false;
            } else {
                m_threadHealth[name] = true;
            }
        }
        return allHealthy;
    }

    /**
     * @brief Get list of unhealthy thread names
     */
    std::vector<std::string> getUnhealthyThreads() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> unhealthy;
        for (const auto& [name, healthy] : m_threadHealth) {
            if (!healthy) {
                unhealthy.push_back(name);
            }
        }
        return unhealthy;
    }

    /**
     * @brief Get formatted health report for dashboard display
     */
    std::string getHealthReport() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ostringstream oss;
        auto now = std::chrono::steady_clock::now();

        oss << "THREAD HEALTH MONITOR\n";
        oss << std::string(40, '-') << "\n";

        for (const auto& [name, healthy] : m_threadHealth) {
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

    /**
     * @brief Check if a specific thread is healthy
     */
    bool isThreadHealthy(const std::string& threadName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_threadHealth.find(threadName);
        return it != m_threadHealth.end() && it->second;
    }

    /**
     * @brief Get number of registered threads
     */
    size_t getRegisteredThreadCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_threadHealth.size();
    }

    /**
     * @brief Get watchdog uptime as formatted string
     */
    std::string getUptime() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
        int hours = static_cast<int>(elapsed / 3600);
        int minutes = static_cast<int>((elapsed % 3600) / 60);
        int seconds = static_cast<int>(elapsed % 60);
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setw(2) << minutes << ":"
            << std::setw(2) << seconds;
        return oss.str();
    }

    /** @brief Get timeout value */
    int getTimeoutMs() const { return m_timeoutMs; }

    /** @brief Set timeout value */
    void setTimeoutMs(int ms) { m_timeoutMs = ms; }

private:
    int m_timeoutMs;
    std::chrono::steady_clock::time_point m_startTime;
    std::map<std::string, std::chrono::steady_clock::time_point> m_lastHeartbeat;
    std::map<std::string, bool> m_threadHealth;
    std::map<std::string, uint64_t> m_heartbeatCount;
    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
