#include "sensors/BatterySensor.hpp"
#include <iomanip>
#include <sstream>

namespace VehicleSystem {

// This function provides the implementation for constructor
BatterySensor::BatterySensor(SensorID id)
    : Sensor(id, "Battery Voltage", SensorType::BATTERY_VOLTAGE) {
    m_currentValue = 12.5; // Nominal battery voltage
}

// This function provides the implementation for update
void BatterySensor::update() {
    double delta = Utils::getRandomDouble(-0.5, 0.3);
    m_currentValue += delta;
    if (m_currentValue < 9.0) m_currentValue = 9.0;
    if (m_currentValue > 15.0) m_currentValue = 15.0;
}

// This function provides the implementation for getValueString
std::string BatterySensor::getValueString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << m_currentValue;
    return oss.str();
}

// This function provides the implementation for display
std::string BatterySensor::display() const {
    std::ostringstream oss;
    oss << std::setw(22) << std::left << m_name << " "
        << std::setw(8) << std::right << getValueString()
        << " " << getUnit();
    return oss.str();
}

// This function provides the implementation for clone
std::unique_ptr<Sensor> BatterySensor::clone() const {
    return std::make_unique<BatterySensor>(*this);
}

} // namespace VehicleSystem
