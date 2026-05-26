#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Tire pressure sensor (20 - 40 PSI range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class TirePressureSensor : public Sensor {
public:
    explicit TirePressureSensor(SensorID id = 4);

    TirePressureSensor(const TirePressureSensor& other) = default;
    TirePressureSensor(TirePressureSensor&& other) noexcept = default;
    ~TirePressureSensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return "PSI"; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;
};

} // namespace VehicleSystem
