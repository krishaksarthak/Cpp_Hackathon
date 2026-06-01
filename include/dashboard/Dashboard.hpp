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
 * 
 * @author Member 3 (Dashboard Developer)
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
                 const std::string& activeProfile);

    void setRefreshRate(int ms) { m_refreshRateMs = ms; }
    int getRefreshRate() const { return m_refreshRateMs; }

private:
    void clearScreen();
    void displayHeader(std::ostringstream& oss, const std::string& profile,
                       const VehicleStatistics& stats);
    void displaySensors(std::ostringstream& oss,
                        const std::vector<std::unique_ptr<Sensor>>& sensors);
    void displayAlerts(std::ostringstream& oss,
                       const AlertManager& alertManager);
    void displayDTCs(std::ostringstream& oss, const DTCManager& dtcManager);
    void displayStatistics(std::ostringstream& oss,
                           const VehicleStatistics& stats);
    void displayWatchdog(std::ostringstream& oss, const Watchdog& watchdog);
    void displayFooter(std::ostringstream& oss);
    void logTelemetryJson(const std::vector<std::unique_ptr<Sensor>>& sensors,
                          const AlertManager& alertManager,
                          const DTCManager& dtcManager,
                          const std::string& activeProfile);

    /** @brief Determine status label based on sensor type and value */
    std::string getSensorStatus(const Sensor* sensor) const;

    int m_refreshRateMs;
    std::mutex m_displayMutex;
};

} // namespace VehicleSystem
