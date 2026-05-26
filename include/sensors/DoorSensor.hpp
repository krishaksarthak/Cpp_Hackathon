#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Door status sensor (OPEN/CLOSED binary state).
 * Demonstrates: Inheritance, polymorphism, enum usage.
 */
class DoorSensor : public Sensor {
public:
    explicit DoorSensor(SensorID id = 5)
        : Sensor(id, "Door Status", SensorType::DOOR_STATUS),
          m_doorState(DoorState::CLOSED) {
        m_currentValue = 0.0; // 0 = CLOSED, 1 = OPEN
    }

    DoorSensor(const DoorSensor& other) = default;
    DoorSensor(DoorSensor&& other) noexcept = default;
    ~DoorSensor() override = default;

    void update() override {
        // 10% chance of state change each update
        if (Utils::getRandomInt(1, 100) <= 10) {
            m_doorState = (m_doorState == DoorState::CLOSED)
                          ? DoorState::OPEN : DoorState::CLOSED;
            m_currentValue = (m_doorState == DoorState::OPEN) ? 1.0 : 0.0;
        }
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        return (m_doorState == DoorState::OPEN) ? "OPEN" : "CLOSED";
    }

    std::string getUnit() const override { return ""; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(12) << std::right << getValueString();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<DoorSensor>(*this);
    }

    DoorState getDoorState() const { return m_doorState; }
    bool isOpen() const { return m_doorState == DoorState::OPEN; }

private:
    DoorState m_doorState;
};

} // namespace VehicleSystem
