#pragma once

#include "sensors/Sensor.hpp"

namespace VehicleSystem {

/**
 * @brief Battery voltage sensor (9V - 15V range).
 * Demonstrates: Inheritance, polymorphism, override specifier.
 */
class BatterySensor : public Sensor {
public:
    explicit BatterySensor(SensorID id = 2)
        : Sensor(id, "Battery Voltage", SensorType::BATTERY_VOLTAGE) {
        m_currentValue = 12.6; // Nominal battery voltage
    }

    BatterySensor(const BatterySensor& other) = default;
    BatterySensor(BatterySensor&& other) noexcept = default;
    ~BatterySensor() override = default;

    void update() override {
        double delta = Utils::getRandomDouble(-0.5, 0.3);
        m_currentValue += delta;
        if (m_currentValue < 9.0) m_currentValue = 9.0;
        if (m_currentValue > 15.0) m_currentValue = 15.0;
    }

    double getValue() const override { return m_currentValue; }

    std::string getValueString() const override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << m_currentValue;
        return oss.str();
    }

    std::string getUnit() const override { return "V"; }

    std::string display() const override {
        std::ostringstream oss;
        oss << std::setw(22) << std::left << m_name << " "
            << std::setw(8) << std::right << getValueString()
            << " " << getUnit();
        return oss.str();
    }

    std::unique_ptr<Sensor> clone() const override {
        return std::make_unique<BatterySensor>(*this);
    }
};

} // namespace VehicleSystem
