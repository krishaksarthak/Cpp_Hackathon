#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Battery voltage sensor (9V - 15V range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class BatterySensor : public Sensor {
public:
    explicit BatterySensor(SensorID id = 2);

    BatterySensor(const BatterySensor& other) = default;
    BatterySensor(BatterySensor&& other) noexcept = default;
    ~BatterySensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return "V"; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;
};

} // namespace VehicleSystem
