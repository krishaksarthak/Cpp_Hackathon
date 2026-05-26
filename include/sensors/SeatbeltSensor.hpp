#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Seatbelt status sensor (LOCKED/UNLOCKED binary state).
 * Demonstrates: Inheritance, polymorphism, enum usage.
 */
class SeatbeltSensor : public Sensor {
public:
    explicit SeatbeltSensor(SensorID id = 6);

    SeatbeltSensor(const SeatbeltSensor& other) = default;
    SeatbeltSensor(SeatbeltSensor&& other) noexcept = default;
    ~SeatbeltSensor() override = default;

    void update() override;

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override;

    std::string getUnit() const override { return ""; }

    std::string display() const override;

    std::unique_ptr<Sensor> clone() const override;

    SeatbeltState getSeatbeltState() const { return m_seatbeltState; }
    bool isLocked() const { return m_seatbeltState == SeatbeltState::LOCKED; }

private:
    SeatbeltState m_seatbeltState;
};

} // namespace VehicleSystem
