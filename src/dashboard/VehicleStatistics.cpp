#include "dashboard/VehicleStatistics.hpp"

namespace VehicleSystem {

void VehicleStatistics::recordSpeed(double speed) {
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

void VehicleStatistics::recordTemperature(double temp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_peakTemperature = std::max(m_peakTemperature, temp);
    m_temperatureHistory.push_back(temp);
    if (m_temperatureHistory.size() > 300) {
        m_temperatureHistory.erase(m_temperatureHistory.begin());
    }
}

void VehicleStatistics::recordBatteryVoltage(double voltage) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minBatteryVoltage = std::min(m_minBatteryVoltage, voltage);
}

void VehicleStatistics::recordTirePressure(double pressure) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minTirePressure = std::min(m_minTirePressure, pressure);
}

void VehicleStatistics::recordAlert(AlertSeverity severity, const std::string& type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_totalAlerts;
    m_alertFrequency[type]++;
    m_severityCount[severityToString(severity)]++;
}

double VehicleStatistics::getAverageSpeed() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_speedHistory.empty()) return 0.0;
    // STL algorithm: std::accumulate with lambda
    double sum = std::accumulate(m_speedHistory.begin(), m_speedHistory.end(), 0.0,
        [](double acc, double val) { return acc + val; });
    return sum / static_cast<double>(m_speedHistory.size());
}

double VehicleStatistics::getAverageTemperature() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_temperatureHistory.empty()) return 0.0;
    double sum = std::accumulate(m_temperatureHistory.begin(),
                                  m_temperatureHistory.end(), 0.0);
    return sum / static_cast<double>(m_temperatureHistory.size());
}

double VehicleStatistics::getPeakTemperature() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_peakTemperature;
}

double VehicleStatistics::getMinBatteryVoltage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minBatteryVoltage;
}

double VehicleStatistics::getMinTirePressure() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minTirePressure;
}

size_t VehicleStatistics::getTotalAlerts() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalAlerts;
}

double VehicleStatistics::getTotalDistance() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalDistance;
}

std::map<std::string, int> VehicleStatistics::getAlertFrequency() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alertFrequency;
}

std::string VehicleStatistics::getMostFrequentAlert() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_alertFrequency.empty()) return "None";

    // STL algorithm: std::max_element with lambda
    auto maxIt = std::max_element(m_alertFrequency.begin(), m_alertFrequency.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    return maxIt->first + " (" + std::to_string(maxIt->second) + "x)";
}

std::string VehicleStatistics::getUptime() const {
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

void VehicleStatistics::reset() {
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

std::string VehicleStatistics::getFormattedStats() const {
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

} // namespace VehicleSystem
