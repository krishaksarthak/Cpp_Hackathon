#include "sensors/EngineTemperatureSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

EngineTemperatureSensor::EngineTemperatureSensor(SensorID id)
    : Sensor(id, "Engine Temperature", SensorType::ENGINE_TEMPERATURE) {
    m_currentValue = 85.0; // Normal starting temp
}

void EngineTemperatureSensor::update() {
    // Simulate realistic temperature fluctuations
    double delta = Utils::getRandomDouble(-3.0, 4.0);
    m_currentValue += delta;
    // Clamp to realistic range
    if (m_currentValue < 60.0) m_currentValue = 60.0;
    if (m_currentValue > 130.0) m_currentValue = 130.0;
}

std::string EngineTemperatureSensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << m_currentValue;
    return oss.str();
}

std::string EngineTemperatureSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

std::unique_ptr<Sensor> EngineTemperatureSensor::clone() const {
    return std::make_unique<EngineTemperatureSensor>(*this);
}

} // namespace VehicleSystem
