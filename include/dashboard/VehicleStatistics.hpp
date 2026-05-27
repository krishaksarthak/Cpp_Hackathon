#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include "common/Types.hpp"
#include "common/SensorDataBuffer.hpp"

namespace VehicleSystem {

/**
 * @brief Tracks and computes vehicle statistics over time.
 * 
 * Demonstrates: STL algorithms (accumulate, max_element, sort),
 * lambda expressions, chrono, map/vector usage.
 * 
 * BONUS FEATURE: Vehicle Statistics Dashboard
 */
class VehicleStatistics {
public:
    VehicleStatistics()
        : m_totalDistance(0.0), m_lastSpeedRecordTime(std::chrono::steady_clock::now()),
          m_peakTemperature(0.0), m_minBatteryVoltage(15.0), m_minTirePressure(40.0),
          m_totalAlerts(0), m_startTime(std::chrono::steady_clock::now()) {}

    ~VehicleStatistics() = default;

    // --- Recording methods (called from sensor/monitoring threads) ---

    void recordSpeed(double speed);
    void recordTemperature(double temp);
    void recordBatteryVoltage(double voltage);
    void recordTirePressure(double pressure);
    void recordAlert(AlertSeverity severity, const std::string& type);

    // --- Computed statistics (use STL algorithms + lambdas) ---

    double getAverageSpeed() const;
    double getAverageTemperature() const;
    double getPeakTemperature() const;
    double getMinBatteryVoltage() const;
    double getMinTirePressure() const;
    size_t getTotalAlerts() const;
    double getTotalDistance() const;
    std::map<std::string, int> getAlertFrequency() const;

    /** @brief Get the most frequently triggered alert type (uses STL max_element + lambda) */
    std::string getMostFrequentAlert() const;

    /** @brief Get uptime as formatted string HH:MM:SS */
    std::string getUptime() const;

    /** @brief Reset all statistics */
    void reset();

    /** @brief Get formatted statistics string for dashboard display */
    std::string getFormattedStats() const;

private:
    // Speed tracking — uses SensorDataBuffer<double> template class
    SensorDataBuffer<double> m_speedHistory{500};
    double m_totalDistance;
    std::chrono::steady_clock::time_point m_lastSpeedRecordTime;

    // Temperature tracking — uses SensorDataBuffer<double> template class
    SensorDataBuffer<double> m_temperatureHistory{500};
    double m_peakTemperature;

    // Battery tracking
    double m_minBatteryVoltage;

    // Tire tracking
    double m_minTirePressure;

    // Alert tracking
    size_t m_totalAlerts;
    std::map<std::string, int> m_alertFrequency;
    std::map<std::string, int> m_severityCount;
    std::set<std::string> m_uniqueAlertTypes; // Demonstrates std::set

    // Timing
    std::chrono::steady_clock::time_point m_startTime;

    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
