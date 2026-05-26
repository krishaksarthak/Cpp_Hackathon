#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Vehicle speed sensor (0 - 200 km/h range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class SpeedSensor : public Sensor {
public:
    explicit SpeedSensor(SensorID id = 3)
        : Sensor(id, "Vehicle Speed", SensorType::VEHICLE_SPEED) {
        m_currentValue = 0.0;
    }

    SpeedSensor(const SpeedSensor& other) = default;
    SpeedSensor(SpeedSensor&& other) noexcept = default;
    ~SpeedSensor() override = default;

    void update() override {
        double delta = Utils::getRandomDouble(-15.0, 18.0);
        m_currentValue += delta;
        if (m_currentValue < 0.0) m_currentValue = 0.0;
        if (m_currentValue > 200.0) m_currentValue = 200.0;
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << m_currentValue;
        return oss.str();
    }

    std::string getUnit() const override { return "km/h"; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(8) << std::right << getValueString()
            << " " << getUnit();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<SpeedSensor>(*this);
    }
};

} // namespace VehicleSystem
