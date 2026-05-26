#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Tire pressure sensor (20 - 40 PSI range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class TirePressureSensor : public Sensor {
public:
    explicit TirePressureSensor(SensorID id = 4)
        : Sensor(id, "Tire Pressure", SensorType::TIRE_PRESSURE) {
        m_currentValue = 32.0; // Nominal tire pressure
    }

    TirePressureSensor(const TirePressureSensor& other) = default;
    TirePressureSensor(TirePressureSensor&& other) noexcept = default;
    ~TirePressureSensor() override = default;

    void update() override {
        double delta = Utils::getRandomDouble(-1.5, 1.0);
        m_currentValue += delta;
        if (m_currentValue < 20.0) m_currentValue = 20.0;
        if (m_currentValue > 40.0) m_currentValue = 40.0;
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << m_currentValue;
        return oss.str();
    }

    std::string getUnit() const override { return "PSI"; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(8) << std::right << getValueString()
            << " " << getUnit();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<TirePressureSensor>(*this);
    }
};

} // namespace VehicleSystem
