#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "common/Types.hpp"
#include "common/Utils.hpp"

namespace VehicleSystem {

/**
 * @brief Represents a vehicle alert/warning condition.
 * 
 * Demonstrates: Operator overloading (<<, ==, <, !=, >),
 * static members, copy/move semantics, RAII.
 */
class Alert {
public:
    /**
     * @brief Construct a new Alert (auto-generates ID and timestamp)
     */
    Alert(AlertSeverity severity, const std::string& message,
          SensorType source, double sensorValue,
          ASILLevel asil = ASILLevel::QM)
        : m_id(s_nextId++), m_severity(severity), m_message(message),
          m_source(source), m_sensorValue(sensorValue),
          m_timestamp(std::chrono::system_clock::now()),
          m_active(true), m_asilLevel(asil) {}

    /**
     * @brief Construct with explicit ID (for deserialization)
     */
    Alert(AlertID id, AlertSeverity severity, const std::string& message,
          SensorType source, double sensorValue,
          ASILLevel asil = ASILLevel::QM)
        : m_id(id), m_severity(severity), m_message(message),
          m_source(source), m_sensorValue(sensorValue),
          m_timestamp(std::chrono::system_clock::now()),
          m_active(true), m_asilLevel(asil) {
        if (id >= s_nextId) s_nextId = id + 1;
    }

    // Copy constructor
    Alert(const Alert& other) = default;

    // Move constructor
    Alert(Alert&& other) noexcept = default;

    // Copy assignment
    Alert& operator=(const Alert& other) = default;

    // Move assignment
    Alert& operator=(Alert&& other) noexcept = default;

    ~Alert() = default;

    // --- Getters ---
    AlertID getId() const { return m_id; }
    AlertSeverity getSeverity() const { return m_severity; }
    std::string getMessage() const { return m_message; }
    SensorType getSource() const { return m_source; }
    double getSensorValue() const { return m_sensorValue; }
    Timestamp getTimestamp() const { return m_timestamp; }
    bool isActive() const { return m_active; }

    /** @brief Get ISO 26262 ASIL level assigned to this alert */
    ASILLevel getASIL() const { return m_asilLevel; }

    /** @brief Get ASIL level as string (e.g. "ASIL C") */
    std::string getASILString() const { return asilToString(m_asilLevel); }

    /** @brief Deactivate this alert (resolved) */
    void deactivate() { m_active = false; }

    /** @brief Get formatted timestamp string */
    std::string getTimestampString() const {
        return Utils::timestampToString(m_timestamp);
    }

    /** @brief Get severity as colored/formatted string */
    std::string getSeverityString() const {
        return severityToString(m_severity);
    }

    // --- Operator Overloading ---

    /** @brief Equality comparison (by ID) */
    bool operator==(const Alert& other) const {
        return m_id == other.m_id;
    }

    /** @brief Inequality comparison */
    bool operator!=(const Alert& other) const {
        return !(*this == other);
    }

    /** @brief Less-than comparison (by severity, then timestamp) */
    bool operator<(const Alert& other) const {
        if (m_severity != other.m_severity) {
            return static_cast<int>(m_severity) < static_cast<int>(other.m_severity);
        }
        return m_timestamp < other.m_timestamp;
    }

    /** @brief Greater-than comparison */
    bool operator>(const Alert& other) const {
        return other < *this;
    }

    /** @brief Stream insertion operator for formatted output */
    friend std::ostream& operator<<(std::ostream& os, const Alert& alert) {
        os << "[" << std::setw(8) << alert.getSeverityString() << "] "
           << "[" << alert.getTimestampString() << "] "
           << "[" << std::setw(6) << alert.getASILString() << "] "
           << alert.m_message;
        if (alert.m_sensorValue != 0.0) {
            os << " (Value: " << std::fixed << std::setprecision(1)
               << alert.m_sensorValue << ")";
        }
        return os;
    }

    /** @brief Convert to string representation */
    std::string toString() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    // --- Static Members ---
    static AlertID getNextId() { return s_nextId; }
    static void resetIdCounter() { s_nextId = 1; }

private:
    static AlertID s_nextId;

    AlertID m_id;
    AlertSeverity m_severity;
    std::string m_message;
    SensorType m_source;
    double m_sensorValue;
    Timestamp m_timestamp;
    bool m_active;
    ASILLevel m_asilLevel;  ///< ISO 26262 safety integrity level for this alert
};

} // namespace VehicleSystem
