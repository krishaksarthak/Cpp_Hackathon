#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "common/Types.hpp"

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
    explicit Watchdog(int timeoutMs = 5000);

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
    void registerThread(const std::string& threadName);

    /**
     * @brief Record a heartbeat from a thread (must be called periodically)
     * @param threadName Name of the thread sending heartbeat
     */
    void heartbeat(const std::string& threadName);

    /**
     * @brief Check health of all registered threads
     * @return true if all threads are healthy
     */
    bool checkHealth();

    /**
     * @brief Get list of unhealthy thread names
     */
    std::vector<std::string> getUnhealthyThreads() const;

    /**
     * @brief Get formatted health report for dashboard display
     */
    std::string getHealthReport() const;

    /**
     * @brief Check if a specific thread is healthy
     */
    bool isThreadHealthy(const std::string& threadName) const;

    /**
     * @brief Get number of registered threads
     */
    size_t getRegisteredThreadCount() const;

    /**
     * @brief Get watchdog uptime as formatted string
     */
    std::string getUptime() const;

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
