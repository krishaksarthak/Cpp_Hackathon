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
 * 
 * @author Member 1 (Architecture Lead)
 */
class Sensor {
public:
    /**
     * @brief Construct a new Sensor object (RAII - increments count)
     */
    Sensor(SensorID id, const std::string& name, SensorType type);

    /**
     * @brief Copy constructor (demonstrates copy semantics)
     */
    Sensor(const Sensor& other);

    /**
     * @brief Move constructor (demonstrates move semantics)
     */
    Sensor(Sensor&& other) noexcept;

    /**
     * @brief Virtual destructor (RAII - decrements count)
     */
    virtual ~Sensor();

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
    void performUpdate();

    SensorID getId() const { return m_id; }
    std::string getName() const { return m_name; }
    SensorType getType() const { return m_type; }
    Timestamp getLastUpdateTime() const { return m_lastUpdateTime; }
    bool isHealthy() const { return m_healthy; }
    uint64_t getUpdateCount() const { return m_updateCount; }

    /** @brief Get status string for dashboard display */
    std::string getStatusString() const;

    // --- Static Members ---
    static int getSensorCount();

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
