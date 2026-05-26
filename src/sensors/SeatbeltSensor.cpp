#include "sensors/SeatbeltSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

SeatbeltSensor::SeatbeltSensor(SensorID id)
    : Sensor(id, "Seatbelt Status", SensorType::SEATBELT_STATUS),
      m_seatbeltState(SeatbeltState::LOCKED) {
    m_currentValue = 1.0; // 1 = LOCKED, 0 = UNLOCKED
}

void SeatbeltSensor::update() {
    // 8% chance of state change each update
    if (Utils::getRandomInt(1, 100) <= 8) {
        m_seatbeltState = (m_seatbeltState == SeatbeltState::LOCKED)
                          ? SeatbeltState::UNLOCKED : SeatbeltState::LOCKED;
        m_currentValue = (m_seatbeltState == SeatbeltState::LOCKED) ? 1.0 : 0.0;
    }
}

std::string SeatbeltSensor::getValueString() const {
    return (m_seatbeltState == SeatbeltState::LOCKED) ? "LOCKED" : "UNLOCKED";
}

std::string SeatbeltSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(12) << std::right << getValueString();
    return oss.str();
}

std::unique_ptr<Sensor> SeatbeltSensor::clone() const {
    return std::make_unique<SeatbeltSensor>(*this);
}

} // namespace VehicleSystem
