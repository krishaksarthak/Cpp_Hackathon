#include "sensors/DoorSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

DoorSensor::DoorSensor(SensorID id)
    : Sensor(id, "Door Status", SensorType::DOOR_STATUS),
      m_doorState(DoorState::CLOSED) {
    m_currentValue = 0.0; // 0 = CLOSED, 1 = OPEN
}

// This function provides the implementation for update
void DoorSensor::update() {
    // 10% chance of state change each update
    if (Utils::getRandomInt(1, 100) <= 10) {
        m_doorState = (m_doorState == DoorState::CLOSED)
                      ? DoorState::OPEN : DoorState::CLOSED;
        m_currentValue = (m_doorState == DoorState::OPEN) ? 1.0 : 0.0;
    }
}

// This function provides the implementation for getValueString
std::string DoorSensor::getValueString() const {
    return (m_doorState == DoorState::OPEN) ? "OPEN" : "CLOSED";
}

// This function provides the implementation for display
std::string DoorSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(12) << std::right << getValueString();
    return oss.str();
}

// This function provides the implementation for clone
std::unique_ptr<Sensor> DoorSensor::clone() const {
    return std::make_unique<DoorSensor>(*this);
}

} // namespace VehicleSystem
