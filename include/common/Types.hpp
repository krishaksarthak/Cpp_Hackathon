#pragma once

#include <string>
#include <chrono>
#include <memory>
#include "common/Win32Threads.hpp"

namespace VehicleSystem {

// Type aliases
using Timestamp = std::chrono::system_clock::time_point;
using Duration = std::chrono::milliseconds;
using SensorID = uint32_t;
using AlertID = uint32_t;

// Enumerations
enum class AlertSeverity {
    INFO,
    WARNING,
    CRITICAL
};

enum class SensorType {
    ENGINE_TEMPERATURE,
    BATTERY_VOLTAGE,
    VEHICLE_SPEED,
    TIRE_PRESSURE,
    DOOR_STATUS,
    SEATBELT_STATUS
};

enum class DoorState {
    OPEN,
    CLOSED
};

enum class SeatbeltState {
    LOCKED,
    UNLOCKED
};

// String conversion helpers
/**
 * @brief Converts an AlertSeverity enum value to its string representation.
 * @param severity The AlertSeverity value to convert.
 * @return std::string The string representation of the severity.
 */
inline std::string severityToString(AlertSeverity severity) {
    switch(severity) {
        case AlertSeverity::INFO: return "INFO";
        case AlertSeverity::WARNING: return "WARNING";
        case AlertSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Converts a SensorType enum value to its string representation.
 * @param type The SensorType value to convert.
 * @return std::string The string representation of the sensor type.
 */
inline std::string sensorTypeToString(SensorType type) {
    switch(type) {
        case SensorType::ENGINE_TEMPERATURE: return "Engine Temperature";
        case SensorType::BATTERY_VOLTAGE: return "Battery Voltage";
        case SensorType::VEHICLE_SPEED: return "Vehicle Speed";
        case SensorType::TIRE_PRESSURE: return "Tire Pressure";
        case SensorType::DOOR_STATUS: return "Door Status";
        case SensorType::SEATBELT_STATUS: return "Seatbelt Status";
        default: return "UNKNOWN";
    }
}

} // namespace VehicleSystem