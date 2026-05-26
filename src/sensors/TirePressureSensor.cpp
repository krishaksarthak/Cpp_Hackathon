#include "sensors/TirePressureSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

TirePressureSensor::TirePressureSensor(SensorID id)
    : Sensor(id, "Tire Pressure", SensorType::TIRE_PRESSURE) {
    m_currentValue = 32.0; // Nominal tire pressure
}

void TirePressureSensor::update() {
    double delta = Utils::getRandomDouble(-1.5, 1.0);
    m_currentValue += delta;
    if (m_currentValue < 20.0) m_currentValue = 20.0;
    if (m_currentValue > 40.0) m_currentValue = 40.0;
}

std::string TirePressureSensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << m_currentValue;
    return oss.str();
}

std::string TirePressureSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

std::unique_ptr<Sensor> TirePressureSensor::clone() const {
    return std::make_unique<TirePressureSensor>(*this);
}

} // namespace VehicleSystem
