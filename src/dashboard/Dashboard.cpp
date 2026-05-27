#include "dashboard/Dashboard.hpp"
#include "common/Utils.hpp"
#include <cstdlib>
#include <fstream>

namespace VehicleSystem {

void Dashboard::display(const std::vector<std::unique_ptr<Sensor>>& sensors,
                 const AlertManager& alertManager,
                 const DTCManager& dtcManager,
                 const VehicleStatistics& stats,
                 const Watchdog& watchdog,
                 const std::string& activeProfile) {
    std::lock_guard<std::mutex> lock(m_displayMutex);
    std::ostringstream oss;

    displayHeader(oss, activeProfile, stats);
    displaySensors(oss, sensors);
    displayAlerts(oss, alertManager);
    displayDTCs(oss, dtcManager);
    displayStatistics(oss, stats);
    displayWatchdog(oss, watchdog);
    displayFooter(oss);

    // Clear screen and print (platform-independent)
    clearScreen();
    std::string output = oss.str();
    std::cout << output << std::flush;

    // Log the exact terminal output to the logs folder
    std::ofstream termLog("logs/terminal_output.log", std::ios::out | std::ios::app);
    if (termLog.is_open()) {
        termLog << output << "\n";
    }
}

void Dashboard::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Dashboard::displayHeader(std::ostringstream& oss, const std::string& profile,
                   const VehicleStatistics& stats) {
    oss << "\n";
    oss << "========================================\n";
    oss << "   VEHICLE MONITORING SYSTEM\n";
    oss << "========================================\n";
    oss << "Status: RUNNING\n";
    std::string colorCode = "\033[0m"; // default
    if (profile == "eco_mode") colorCode = "\033[32m"; // Green
    else if (profile == "sport_mode") colorCode = "\033[31m"; // Red
    else if (profile == "comfort_mode") colorCode = "\033[36m"; // Cyan

    oss << "Uptime:  " << stats.getUptime() << "\n";
    oss << "Profile: " << colorCode << profile << "\033[0m\n";
    oss << "========================================\n";
}

void Dashboard::displaySensors(std::ostringstream& oss,
                    const std::vector<std::unique_ptr<Sensor>>& sensors) {
    oss << "\n SENSOR READINGS\n";
    oss << std::string(40, '-') << "\n";

    for (const auto& sensor : sensors) {
        std::string status = getSensorStatus(sensor.get());
        oss << " " << sensor->display();
        oss << "  [" << status << "]\n";
    }
}

void Dashboard::displayAlerts(std::ostringstream& oss,
                   const AlertManager& alertManager) {
    auto activeAlerts = alertManager.getActiveAlerts();
    oss << "\n ACTIVE ALERTS: " << activeAlerts.size() << "\n";
    oss << std::string(40, '-') << "\n";

    if (activeAlerts.empty()) {
        oss << " No active alerts\n";
    } else {
        for (const auto& alert : activeAlerts) {
            oss << " " << alert << "\n";
        }
    }
}

void Dashboard::displayDTCs(std::ostringstream& oss, const DTCManager& dtcManager) {
    auto dtcs = dtcManager.getActiveDTCs();
    if (!dtcs.empty()) {
        oss << "\n DTC CODES: " << dtcs.size() << "\n";
        oss << std::string(40, '-') << "\n";
        for (const auto& dtc : dtcs) {
            oss << " " << dtc.format() << "\n";
        }
    }
}

void Dashboard::displayStatistics(std::ostringstream& oss,
                       const VehicleStatistics& stats) {
    oss << "\n STATISTICS\n";
    oss << std::string(40, '-') << "\n";
    oss << " " << stats.getFormattedStats();
}

void Dashboard::displayWatchdog(std::ostringstream& oss, const Watchdog& watchdog) {
    oss << "\n " << watchdog.getHealthReport();
}

void Dashboard::displayFooter(std::ostringstream& oss) {
    oss << "\n========================================\n";
    oss << "Last Updated: " << Utils::getCurrentTimestamp() << "\n";
    oss << "Press Ctrl+C or 'q' to exit | 'p' to switch profile\n";
    oss << "========================================\n";
}

std::string Dashboard::getSensorStatus(const Sensor* sensor) const {
    if (!sensor->isHealthy()) return "OFFLINE";

    switch (sensor->getType()) {
        case SensorType::ENGINE_TEMPERATURE:
            if (sensor->getValue() > 110) return "CRITICAL";
            if (sensor->getValue() > 100) return "WARNING";
            return "NORMAL";
        case SensorType::BATTERY_VOLTAGE:
            if (sensor->getValue() < 10.0) return "WARNING";
            return "NORMAL";
        case SensorType::VEHICLE_SPEED:
            if (sensor->getValue() > 120) return "WARNING";
            return "NORMAL";
        case SensorType::TIRE_PRESSURE:
            if (sensor->getValue() < 25) return "WARNING";
            return "NORMAL";
        case SensorType::DOOR_STATUS:
            return (sensor->getValue() > 0.5) ? "OPEN" : "NORMAL";
        case SensorType::SEATBELT_STATUS:
            return (sensor->getValue() < 0.5) ? "UNLOCKED" : "NORMAL";
        default:
            return "UNKNOWN";
    }
}

} // namespace VehicleSystem
