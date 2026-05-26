#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Vehicle speed sensor (0 - 200 km/h range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class SpeedSensor : public Sensor {
public:
    explicit SpeedSensor(SensorID id = 3);

    SpeedSensor(const SpeedSensor& other) = default;
    SpeedSensor(SpeedSensor&& other) noexcept = default;
    ~SpeedSensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return "km/h"; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;
};

} // namespace VehicleSystem
