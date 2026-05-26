#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <iomanip>
#include "common/Types.hpp"
#include "common/Utils.hpp"

namespace VehicleSystem {

/**
 * @brief Abstract base class for all vehicle sensors.
 * 
 * Demonstrates: Inheritance, pure virtual functions, virtual destructor,
 * RAII (sensor count), static members, copy/move semantics.
 * Maps to MCAL (Microcontroller Abstraction Layer) in AUTOSAR.
 */
class Sensor {
public:
    /**
     * @brief Construct a new Sensor object (RAII - increments count)
     */
    Sensor(SensorID id, const std::string& name, SensorType type)
        : m_id(id), m_name(name), m_type(type), m_currentValue(0.0),
          m_lastUpdateTime(std::chrono::system_clock::now()), m_healthy(true),
          m_updateCount(0) {
        ++s_sensorCount;
    }

    /**
     * @brief Copy constructor (demonstrates copy semantics)
     */
    Sensor(const Sensor& other)
        : m_id(other.m_id), m_name(other.m_name), m_type(other.m_type),
          m_currentValue(other.m_currentValue),
          m_lastUpdateTime(other.m_lastUpdateTime), m_healthy(other.m_healthy),
          m_updateCount(other.m_updateCount) {
        ++s_sensorCount;
    }

    /**
     * @brief Move constructor (demonstrates move semantics)
     */
    Sensor(Sensor&& other) noexcept
        : m_id(other.m_id), m_name(std::move(other.m_name)), m_type(other.m_type),
          m_currentValue(other.m_currentValue),
          m_lastUpdateTime(other.m_lastUpdateTime), m_healthy(other.m_healthy),
          m_updateCount(other.m_updateCount) {
        other.m_healthy = false;
        // Note: don't change s_sensorCount since moved-from object still exists
    }

    /**
     * @brief Virtual destructor (RAII - decrements count)
     */
    virtual ~Sensor() {
        --s_sensorCount;
    }

    // --- Pure Virtual Functions (derived classes MUST implement) ---

    /** @brief Update sensor reading with new simulated value */
    virtual void update() = 0;

    /** @brief Get the current sensor value as a double */
    virtual double getValue() const = 0;

    /** @brief Get a formatted string representation of the value */
    virtual std::string getValueString() const = 0;

    /** @brief Get the unit of measurement (°C, V, km/h, PSI, etc.) */
    virtual std::string getUnit() const = 0;

    /** @brief Get a formatted display string for the dashboard */
    virtual std::string display() const = 0;

    /** @brief Clone this sensor (virtual constructor pattern) */
    virtual std::unique_ptr<Sensor> clone() const = 0;

    // --- Concrete Methods ---

    /** @brief Template method: performs update and records metadata */
    void performUpdate() {
        try {
            update();
            m_lastUpdateTime = std::chrono::system_clock::now();
            ++m_updateCount;
            m_healthy = true;
        } catch (const std::exception& e) {
            m_healthy = false;
            throw;  // Re-throw for caller to handle
        }
    }

    SensorID getId() const { return m_id; }
    std::string getName() const { return m_name; }
    SensorType getType() const { return m_type; }
    Timestamp getLastUpdateTime() const { return m_lastUpdateTime; }
    bool isHealthy() const { return m_healthy; }
    uint64_t getUpdateCount() const { return m_updateCount; }

    /** @brief Get status string for dashboard display */
    std::string getStatusString() const {
        return m_healthy ? "ONLINE" : "OFFLINE";
    }

    // --- Static Members ---
    static int getSensorCount() { return s_sensorCount; }

protected:
    SensorID m_id;
    std::string m_name;
    SensorType m_type;
    double m_currentValue;
    Timestamp m_lastUpdateTime;
    bool m_healthy;
    uint64_t m_updateCount;

    /** @brief Static member tracking total sensor instances (RAII) */
    static int s_sensorCount;
};

} // namespace VehicleSystem
