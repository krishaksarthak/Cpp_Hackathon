#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "common/Types.hpp"
#include "sensors/Sensor.hpp"
#include "alerts/AlertManager.hpp"
#include "alerts/DTC.hpp"
#include "dashboard/VehicleStatistics.hpp"
#include "threading/Watchdog.hpp"

namespace VehicleSystem {

/**
 * @brief Dashboard display manager for the vehicle monitoring system.
 * 
 * Renders formatted console output showing:
 * - Current sensor readings with status indicators
 * - Active alerts and alert history
 * - Active DTC codes
 * - Vehicle statistics
 * - Thread health status (via watchdog)
 * 
 * Demonstrates: Formatted I/O, string manipulation, polymorphic access.
 */
class Dashboard {
public:
    Dashboard() : m_refreshRateMs(2000) {}
    ~Dashboard() = default;

    // Delete copy
    Dashboard(const Dashboard&) = delete;
    Dashboard& operator=(const Dashboard&) = delete;

    /**
     * @brief Render the complete dashboard to console.
     */
    void display(const std::vector<std::unique_ptr<Sensor>>& sensors,
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
        std::cout << oss.str() << std::flush;
    }

    void setRefreshRate(int ms) { m_refreshRateMs = ms; }
    int getRefreshRate() const { return m_refreshRateMs; }

private:
    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void displayHeader(std::ostringstream& oss, const std::string& profile,
                       const VehicleStatistics& stats) {
        oss << "\n";
        oss << "========================================\n";
        oss << "   VEHICLE MONITORING SYSTEM\n";
        oss << "========================================\n";
        oss << "Status: RUNNING";
        auto alerts = 0; // Will be updated
        oss << "\n";
        oss << "Uptime:  " << stats.getUptime() << "\n";
        oss << "Profile: " << profile << "\n";
        oss << "========================================\n";
    }

    void displaySensors(std::ostringstream& oss,
                        const std::vector<std::unique_ptr<Sensor>>& sensors) {
        oss << "\n SENSOR READINGS\n";
        oss << std::string(40, '-') << "\n";

        for (const auto& sensor : sensors) {
            std::string status = getSensorStatus(sensor.get());
            oss << " " << sensor->display();
            oss << "  [" << status << "]\n";
        }
    }

    void displayAlerts(std::ostringstream& oss,
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

    void displayDTCs(std::ostringstream& oss, const DTCManager& dtcManager) {
        auto dtcs = dtcManager.getActiveDTCs();
        if (!dtcs.empty()) {
            oss << "\n DTC CODES: " << dtcs.size() << "\n";
            oss << std::string(40, '-') << "\n";
            for (const auto& dtc : dtcs) {
                oss << " " << dtc.format() << "\n";
            }
        }
    }

    void displayStatistics(std::ostringstream& oss,
                           const VehicleStatistics& stats) {
        oss << "\n STATISTICS\n";
        oss << std::string(40, '-') << "\n";
        oss << " " << stats.getFormattedStats();
    }

    void displayWatchdog(std::ostringstream& oss, const Watchdog& watchdog) {
        oss << "\n " << watchdog.getHealthReport();
    }

    void displayFooter(std::ostringstream& oss) {
        oss << "\n========================================\n";
        oss << "Last Updated: " << Utils::getCurrentTimestamp() << "\n";
        oss << "Press Ctrl+C to exit | 'p' to switch profile\n";
        oss << "========================================\n";
    }

    /** @brief Determine status label based on sensor type and value */
    std::string getSensorStatus(const Sensor* sensor) const {
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

    int m_refreshRateMs;
    std::mutex m_displayMutex;
};

} // namespace VehicleSystem
