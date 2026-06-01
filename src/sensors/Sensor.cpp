#include "sensors/Sensor.hpp"

namespace VehicleSystem {

int Sensor::s_sensorCount = 0;

Sensor::Sensor(SensorID id, const std::string& name, SensorType type)
    : m_id(id), m_name(name), m_type(type), m_currentValue(0.0),
      m_lastUpdateTime(std::chrono::system_clock::now()), m_healthy(true),
      m_updateCount(0) {
    ++s_sensorCount;
}

Sensor::Sensor(const Sensor& other)
    : m_id(other.m_id), m_name(other.m_name), m_type(other.m_type),
      m_currentValue(other.m_currentValue),
      m_lastUpdateTime(other.m_lastUpdateTime), m_healthy(other.m_healthy),
      m_updateCount(other.m_updateCount) {
    ++s_sensorCount;
}

Sensor::Sensor(Sensor&& other) noexcept
    : m_id(other.m_id), m_name(std::move(other.m_name)), m_type(other.m_type),
      m_currentValue(other.m_currentValue),
      m_lastUpdateTime(other.m_lastUpdateTime), m_healthy(other.m_healthy),
      m_updateCount(other.m_updateCount) {
    other.m_healthy = false;
}

Sensor::~Sensor() {
    --s_sensorCount;
}

// This function provides the implementation for performUpdate
void Sensor::performUpdate() {
    try {
        if (!m_isForced) {
            update(); 
        }
        m_lastUpdateTime = std::chrono::system_clock::now();
        ++m_updateCount;
        m_healthy = true;
    } catch (const std::exception& e) {
        m_healthy = false;
        throw;
    }
}

// This function provides the implementation for getStatusString
std::string Sensor::getStatusString() const {
    return m_healthy ? "ONLINE" : "OFFLINE";
}

// This function provides the implementation for getSensorCount
int Sensor::getSensorCount() {
    return s_sensorCount;
}

void Sensor::injectTestValue(double value) {
    m_currentValue = value;
    m_isForced = true;
}

void Sensor::clearTestValue() {
    m_isForced = false;
}

} // namespace VehicleSystem
