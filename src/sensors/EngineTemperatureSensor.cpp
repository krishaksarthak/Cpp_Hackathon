#include "sensors/EngineTemperatureSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

// This function provides the implementation for constructor
EngineTemperatureSensor::EngineTemperatureSensor(SensorID id)
    : Sensor(id, "Engine Temperature", SensorType::ENGINE_TEMPERATURE) {
    m_currentValue = 85.0; // Normal starting temp
}

// This function provides the implementation for update
void EngineTemperatureSensor::update() {
    // Simulate realistic temperature fluctuations
    double delta = Utils::getRandomDouble(-3.0, 4.0);
    m_currentValue += delta;
    // Clamp to realistic range
    if (m_currentValue < 60.0) m_currentValue = 60.0;
    if (m_currentValue > 130.0) m_currentValue = 130.0;
}

// This function provides the implementation for getValueString
std::string EngineTemperatureSensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << m_currentValue;
    return oss.str();
}

// This function provides the implementation for display
std::string EngineTemperatureSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

// This function provides the implementation for clone
std::unique_ptr<Sensor> EngineTemperatureSensor::clone() const {
    return std::make_unique<EngineTemperatureSensor>(*this);
}

} // namespace VehicleSystem
