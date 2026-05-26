#include "sensors/SpeedSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

SpeedSensor::SpeedSensor(SensorID id)
    : Sensor(id, "Vehicle Speed", SensorType::VEHICLE_SPEED) {
    m_currentValue = 0.0;
}

void SpeedSensor::update() {
    double delta = Utils::getRandomDouble(-15.0, 18.0);
    m_currentValue += delta;
    if (m_currentValue < 0.0) m_currentValue = 0.0;
    if (m_currentValue > 200.0) m_currentValue = 200.0;
}

std::string SpeedSensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << m_currentValue;
    return oss.str();
}

std::string SpeedSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

std::unique_ptr<Sensor> SpeedSensor::clone() const {
    return std::make_unique<SpeedSensor>(*this);
}

} // namespace VehicleSystem
