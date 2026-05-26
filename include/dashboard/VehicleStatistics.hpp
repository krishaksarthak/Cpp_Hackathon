#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include "common/Types.hpp"

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
        : m_totalDistance(0.0), m_peakTemperature(0.0),
          m_minBatteryVoltage(15.0), m_minTirePressure(40.0),
          m_totalAlerts(0), m_startTime(std::chrono::steady_clock::now()),
          m_lastSpeedRecordTime(std::chrono::steady_clock::now()) {}

    ~VehicleStatistics() = default;

    // --- Recording methods (called from sensor/monitoring threads) ---

    void recordSpeed(double speed) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_speedHistory.push_back(speed);

        // Calculate distance: speed * time since last record
        auto now = std::chrono::steady_clock::now();
        double elapsedHours = std::chrono::duration<double, std::ratio<3600>>(
            now - m_lastSpeedRecordTime).count();
        m_totalDistance += speed * elapsedHours;
        m_lastSpeedRecordTime = now;

        // Keep only last 300 readings (~5 min at 1/sec)
        if (m_speedHistory.size() > 300) {
            m_speedHistory.erase(m_speedHistory.begin());
        }
    }

    void recordTemperature(double temp) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_peakTemperature = std::max(m_peakTemperature, temp);
        m_temperatureHistory.push_back(temp);
        if (m_temperatureHistory.size() > 300) {
            m_temperatureHistory.erase(m_temperatureHistory.begin());
        }
    }

    void recordBatteryVoltage(double voltage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minBatteryVoltage = std::min(m_minBatteryVoltage, voltage);
    }

    void recordTirePressure(double pressure) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minTirePressure = std::min(m_minTirePressure, pressure);
    }

    void recordAlert(AlertSeverity severity, const std::string& type) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_totalAlerts;
        m_alertFrequency[type]++;
        m_severityCount[severityToString(severity)]++;
    }

    // --- Computed statistics (use STL algorithms + lambdas) ---

    double getAverageSpeed() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_speedHistory.empty()) return 0.0;
        // STL algorithm: std::accumulate with lambda
        double sum = std::accumulate(m_speedHistory.begin(), m_speedHistory.end(), 0.0,
            [](double acc, double val) { return acc + val; });
        return sum / static_cast<double>(m_speedHistory.size());
    }

    double getAverageTemperature() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_temperatureHistory.empty()) return 0.0;
        double sum = std::accumulate(m_temperatureHistory.begin(),
                                      m_temperatureHistory.end(), 0.0);
        return sum / static_cast<double>(m_temperatureHistory.size());
    }

    double getPeakTemperature() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_peakTemperature;
    }

    double getMinBatteryVoltage() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_minBatteryVoltage;
    }

    double getMinTirePressure() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_minTirePressure;
    }

    size_t getTotalAlerts() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalAlerts;
    }

    double getTotalDistance() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalDistance;
    }

    std::map<std::string, int> getAlertFrequency() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_alertFrequency;
    }

    /** @brief Get the most frequently triggered alert type (uses STL max_element + lambda) */
    std::string getMostFrequentAlert() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_alertFrequency.empty()) return "None";

        // STL algorithm: std::max_element with lambda
        auto maxIt = std::max_element(m_alertFrequency.begin(), m_alertFrequency.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        return maxIt->first + " (" + std::to_string(maxIt->second) + "x)";
    }

    /** @brief Get uptime as formatted string HH:MM:SS */
    std::string getUptime() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_startTime).count();
        int hours = static_cast<int>(elapsed / 3600);
        int minutes = static_cast<int>((elapsed % 3600) / 60);
        int seconds = static_cast<int>(elapsed % 60);

        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setw(2) << minutes << ":"
            << std::setw(2) << seconds;
        return oss.str();
    }

    /** @brief Reset all statistics */
    void reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_speedHistory.clear();
        m_temperatureHistory.clear();
        m_totalDistance = 0.0;
        m_peakTemperature = 0.0;
        m_minBatteryVoltage = 15.0;
        m_minTirePressure = 40.0;
        m_totalAlerts = 0;
        m_alertFrequency.clear();
        m_severityCount.clear();
        m_startTime = std::chrono::steady_clock::now();
    }

    /** @brief Get formatted statistics string for dashboard display */
    std::string getFormattedStats() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        oss << "Avg Speed:        " << std::setw(8) << getAverageSpeed() << " km/h\n";
        oss << "Peak Temp:        " << std::setw(8) << getPeakTemperature() << " C\n";
        oss << "Avg Temp:         " << std::setw(8) << getAverageTemperature() << " C\n";
        oss << "Min Battery:      " << std::setw(8) << getMinBatteryVoltage() << " V\n";
        oss << "Min Tire Press:   " << std::setw(8) << getMinTirePressure() << " PSI\n";
        oss << "Total Distance:   " << std::setw(8) << getTotalDistance() << " km\n";
        oss << "Total Alerts:     " << std::setw(8) << getTotalAlerts() << "\n";
        oss << "Most Frequent:    " << getMostFrequentAlert() << "\n";
        oss << "Uptime:           " << getUptime() << "\n";
        return oss.str();
    }

private:
    // Speed tracking
    std::vector<double> m_speedHistory;
    double m_totalDistance;
    std::chrono::steady_clock::time_point m_lastSpeedRecordTime;

    // Temperature tracking
    std::vector<double> m_temperatureHistory;
    double m_peakTemperature;

    // Battery tracking
    double m_minBatteryVoltage;

    // Tire tracking
    double m_minTirePressure;

    // Alert tracking
    size_t m_totalAlerts;
    std::map<std::string, int> m_alertFrequency;
    std::map<std::string, int> m_severityCount;

    // Timing
    std::chrono::steady_clock::time_point m_startTime;

    mutable std::mutex m_mutex;
};

} // namespace VehicleSystem
