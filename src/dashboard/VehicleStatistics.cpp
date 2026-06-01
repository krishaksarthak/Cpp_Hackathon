#include "dashboard/VehicleStatistics.hpp"

namespace VehicleSystem {

void VehicleStatistics::recordSpeed(double speed) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // SensorDataBuffer<double>::push() — template method handles circular eviction
    m_speedHistory.push(speed);

    // Calculate distance: speed * time since last record
    auto now = std::chrono::steady_clock::now();
    double elapsedHours = std::chrono::duration<double, std::ratio<3600>>(
        now - m_lastSpeedRecordTime).count();
    m_totalDistance += speed * elapsedHours;
    m_lastSpeedRecordTime = now;
}

void VehicleStatistics::recordTemperature(double temp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_peakTemperature = std::max(m_peakTemperature, temp);
    // SensorDataBuffer<double>::push() — template method
    m_temperatureHistory.push(temp);
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
    m_uniqueAlertTypes.insert(type); // Uses std::set to ensure uniqueness
}

double VehicleStatistics::getAverageSpeed() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    // SensorDataBuffer<double>::average() uses std::accumulate internally
    return m_speedHistory.average();
}

double VehicleStatistics::getAverageTemperature() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    // SensorDataBuffer<double>::average() uses std::accumulate internally
    return m_temperatureHistory.average();
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

std::map<std::string, uint32_t> VehicleStatistics::getAlertFrequency() const {
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
    uint32_t hours = static_cast<uint32_t>(elapsed / 3600);
    uint32_t minutes = static_cast<uint32_t>((elapsed % 3600) / 60);
    uint32_t seconds = static_cast<uint32_t>(elapsed % 60);

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
    m_uniqueAlertTypes.clear();
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
    oss << "Unique Alerts:    " << std::setw(8) << m_uniqueAlertTypes.size() << "\n";
    oss << "Most Frequent:    " << getMostFrequentAlert() << "\n";
    oss << "Uptime:           " << getUptime() << "\n";
    return oss.str();
}

} // namespace VehicleSystem
