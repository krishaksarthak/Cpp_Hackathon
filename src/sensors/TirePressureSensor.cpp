#include "sensors/TirePressureSensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

// This function provides the implementation for constructor
TirePressureSensor::TirePressureSensor(SensorID id)
    : Sensor(id, "Tire Pressure", SensorType::TIRE_PRESSURE) {
    m_currentValue = 30.0; // Nominal tire pressure
}

// This function provides the implementation for update
void TirePressureSensor::update() {
    double delta = Utils::getRandomDouble(-1.5, 1.0);
    m_currentValue += delta;
    if (m_currentValue < 20.0) m_currentValue = 20.0;
    if (m_currentValue > 40.0) m_currentValue = 40.0;
}

// This function provides the implementation for getValueString
std::string TirePressureSensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << m_currentValue;
    return oss.str();
}

// This function provides the implementation for display
std::string TirePressureSensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

// This function provides the implementation for clone
std::unique_ptr<Sensor> TirePressureSensor::clone() const {
    return std::make_unique<TirePressureSensor>(*this);
}

} // namespace VehicleSystem
