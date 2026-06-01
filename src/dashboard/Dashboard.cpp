#include "dashboard/Dashboard.hpp"
#include "common/Utils.hpp"
#include "common/Colors.hpp"
#include <cstdlib>
#include <fstream>
#include <regex>

namespace VehicleSystem {

static std::string stripAnsi(const std::string& input) {
    std::string result;
    bool inEscape = false;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\033') {
            inEscape = true;
        } else if (inEscape) {
            if (input[i] == 'm') {
                inEscape = false;
            }
        } else {
            result += input[i];
        }
    }
    return result;
}

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

    // Log the exact terminal output to the logs folder (stripping ANSI colors)
    std::ofstream termLog("logs/terminal_output.log", std::ios::out | std::ios::app);
    if (termLog.is_open()) {
        termLog << stripAnsi(output) << "\n";
    }
    logTelemetryJson(sensors, alertManager, dtcManager, activeProfile);
}

void Dashboard::logTelemetryJson(const std::vector<std::unique_ptr<Sensor>>& sensors,
                                 const AlertManager& alertManager,
                                 const DTCManager& dtcManager,
                                 const std::string& activeProfile) {
    // Open in append mode
    std::ofstream jsonLog("logs/telemetry.json", std::ios::out | std::ios::app);
    if (!jsonLog.is_open()) return;

    // Default values in case a sensor is missing
    double speed = 0, temp = 0, battery = 0;
    
    // Extract sensor values dynamically via polymorphism
    for (const auto& sensor : sensors) {
        if (sensor->getType() == SensorType::VEHICLE_SPEED) {
            speed = sensor->getValue();
        } else if (sensor->getType() == SensorType::ENGINE_TEMPERATURE) {
            temp = sensor->getValue();
        } else if (sensor->getType() == SensorType::BATTERY_VOLTAGE) {
            battery = sensor->getValue();
        }
    }

    // Write Newline Delimited JSON (NDJSON)
    // std::fixed and std::setprecision keep the JSON numbers clean
    jsonLog << "{"
            << "\"timestamp\":\"" << Utils::getCurrentTimestamp() << "\","
            << "\"profile\":\"" << activeProfile << "\","
            << "\"sensors\":{"
            << "\"speed_kmh\":" << std::fixed << std::setprecision(1) << speed << ","
            << "\"engine_temp_c\":" << temp << ","
            << "\"battery_v\":" << battery 
            << "},"
            << "\"diagnostics\":{"
            << "\"active_alerts\":" << alertManager.getActiveAlertCount() << ","
            << "\"active_dtcs\":" << dtcManager.getActiveDTCCount()
            << "}"
            << "}\n";
            
    jsonLog.flush(); // Ensure it writes to disk immediately (crash-safe)
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
    oss << Colors::CYAN << "========================================\n";
    oss << "   VEHICLE MONITORING SYSTEM\n";
    oss << "========================================\n" << Colors::RESET;
    oss << "Status: " << Colors::GREEN << "RUNNING" << Colors::RESET << "\n";
    std::string colorCode = Colors::RESET;
    if (profile == "eco_mode") colorCode = Colors::GREEN;
    else if (profile == "sport_mode") colorCode = Colors::RED;
    else if (profile == "comfort_mode") colorCode = Colors::CYAN;

    oss << "Uptime:  " << stats.getUptime() << "\n";
    oss << "Profile: " << colorCode << profile << Colors::RESET << "\n";
    oss << Colors::CYAN << "========================================\n" << Colors::RESET;
}

void Dashboard::displaySensors(std::ostringstream& oss,
                    const std::vector<std::unique_ptr<Sensor>>& sensors) {
    oss << Colors::CYAN << "\n SENSOR READINGS\n";
    oss << std::string(40, '-') << "\n" << Colors::RESET;

    for (const auto& sensor : sensors) {
        std::string status = getSensorStatus(sensor.get());
        oss << " " << sensor->display();
        oss << "  [" << status << "]\n";
    }
}

void Dashboard::displayAlerts(std::ostringstream& oss,
                   const AlertManager& alertManager) {
    auto activeAlerts = alertManager.getActiveAlerts();
    oss << Colors::CYAN << "\n ACTIVE ALERTS: " << activeAlerts.size() << "\n";
    oss << std::string(40, '-') << "\n" << Colors::RESET;

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
        oss << Colors::CYAN << "\n DTC CODES: " << dtcs.size() << "\n";
        oss << std::string(40, '-') << "\n" << Colors::RESET;
        for (const auto& dtc : dtcs) {
            oss << " " << dtc.format() << "\n";
        }
    }
}

void Dashboard::displayStatistics(std::ostringstream& oss,
                       const VehicleStatistics& stats) {
    oss << Colors::CYAN << "\n STATISTICS\n";
    oss << std::string(40, '-') << "\n" << Colors::RESET;
    oss << " " << stats.getFormattedStats();
}

void Dashboard::displayWatchdog(std::ostringstream& oss, const Watchdog& watchdog) {
    oss << "\n " << watchdog.getHealthReport();
}

void Dashboard::displayFooter(std::ostringstream& oss) {
    oss << Colors::CYAN << "\n========================================\n";
    oss << "Last Updated: " << Utils::getCurrentTimestamp() << "\n";
    oss << "Press Ctrl+C or 'q' to exit | 'p' to switch profile\n";
    oss << "========================================\n" << Colors::RESET;
}

std::string Dashboard::getSensorStatus(const Sensor* sensor) const {
    if (!sensor->isHealthy()) return Colors::RED + "OFFLINE" + Colors::RESET;

    auto critical = []() { return Colors::RED + "CRITICAL" + Colors::RESET; };
    auto warning = []() { return Colors::YELLOW + "WARNING" + Colors::RESET; };
    auto normal = []() { return Colors::GREEN + "NORMAL" + Colors::RESET; };

    switch (sensor->getType()) {
        case SensorType::ENGINE_TEMPERATURE:
            if (sensor->getValue() > 110) return critical();
            if (sensor->getValue() > 100) return warning();
            return normal();
        case SensorType::BATTERY_VOLTAGE:
            if (sensor->getValue() < 10.0) return warning();
            return normal();
        case SensorType::VEHICLE_SPEED:
            if (sensor->getValue() > 120) return warning();
            return normal();
        case SensorType::TIRE_PRESSURE:
            if (sensor->getValue() < 25) return warning();
            return normal();
        case SensorType::DOOR_STATUS:
            return (sensor->getValue() > 0.5) ? Colors::YELLOW + "OPEN" + Colors::RESET : normal();
        case SensorType::SEATBELT_STATUS:
            return (sensor->getValue() < 0.5) ? Colors::YELLOW + "UNLOCKED" + Colors::RESET : normal();
        default:
            return "UNKNOWN";
    }
}

} // namespace VehicleSystem
