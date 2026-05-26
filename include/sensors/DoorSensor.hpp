#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Door status sensor (OPEN/CLOSED binary state).
 * Demonstrates: Inheritance, polymorphism, enum usage.
 */
class DoorSensor : public Sensor {
public:
    explicit DoorSensor(SensorID id = 5);

    DoorSensor(const DoorSensor& other) = default;
    DoorSensor(DoorSensor&& other) noexcept = default;
    ~DoorSensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return ""; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;

    DoorState getDoorState() const { return m_doorState; }
    bool isOpen() const { return m_doorState == DoorState::OPEN; }

private:
    DoorState m_doorState;
};

} // namespace VehicleSystem
