#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Seatbelt status sensor (LOCKED/UNLOCKED binary state).
 * Demonstrates: Inheritance, polymorphism, enum usage.
 */
class SeatbeltSensor : public Sensor {
public:
    explicit SeatbeltSensor(SensorID id = 6)
        : Sensor(id, "Seatbelt Status", SensorType::SEATBELT_STATUS),
          m_seatbeltState(SeatbeltState::LOCKED) {
        m_currentValue = 1.0; // 1 = LOCKED, 0 = UNLOCKED
    }

    SeatbeltSensor(const SeatbeltSensor& other) = default;
    SeatbeltSensor(SeatbeltSensor&& other) noexcept = default;
    ~SeatbeltSensor() override = default;

    void update() override {
        // 8% chance of state change each update
        if (Utils::getRandomInt(1, 100) <= 8) {
            m_seatbeltState = (m_seatbeltState == SeatbeltState::LOCKED)
                              ? SeatbeltState::UNLOCKED : SeatbeltState::LOCKED;
            m_currentValue = (m_seatbeltState == SeatbeltState::LOCKED) ? 1.0 : 0.0;
        }
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        return (m_seatbeltState == SeatbeltState::LOCKED) ? "LOCKED" : "UNLOCKED";
    }

    std::string getUnit() const override { return ""; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(12) << std::right << getValueString();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<SeatbeltSensor>(*this);
    }

    SeatbeltState getSeatbeltState() const { return m_seatbeltState; }
    bool isLocked() const { return m_seatbeltState == SeatbeltState::LOCKED; }

private:
    SeatbeltState m_seatbeltState;
};

} // namespace VehicleSystem
