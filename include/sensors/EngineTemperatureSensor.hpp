#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Engine temperature sensor (60°C - 130°C range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class EngineTemperatureSensor : public Sensor {
public:
    explicit EngineTemperatureSensor(SensorID id = 1);

    // Copy constructor
    EngineTemperatureSensor(const EngineTemperatureSensor& other) = default;

    // Move constructor
    EngineTemperatureSensor(EngineTemperatureSensor&& other) noexcept = default;

    ~EngineTemperatureSensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return "C"; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;
};

} // namespace VehicleSystem
