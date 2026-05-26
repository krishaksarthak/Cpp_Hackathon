#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Engine temperature sensor (60°C - 130°C range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class EngineTemperatureSensor : public Sensor {
public:
    explicit EngineTemperatureSensor(SensorID id = 1)
        : Sensor(id, "Engine Temperature", SensorType::ENGINE_TEMPERATURE) {
        m_currentValue = 85.0; // Normal starting temp
    }

    // Copy constructor
    EngineTemperatureSensor(const EngineTemperatureSensor& other) = default;

    // Move constructor
    EngineTemperatureSensor(EngineTemperatureSensor&& other) noexcept = default;

    ~EngineTemperatureSensor() override = default;

    void update() override {
        // Simulate realistic temperature fluctuations
        double delta = Utils::getRandomDouble(-3.0, 4.0);
        m_currentValue += delta;
        // Clamp to realistic range
        if (m_currentValue < 60.0) m_currentValue = 60.0;
        if (m_currentValue > 130.0) m_currentValue = 130.0;
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << m_currentValue;
        return oss.str();
    }

    std::string getUnit() const override { return "C"; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(8) << std::right << getValueString()
            << " " << getUnit();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<EngineTemperatureSensor>(*this);
    }
};

} // namespace VehicleSystem
