# Coding Standards

## Naming Conventions

### Classes
```cpp
class SensorManager {};          // PascalCase
class EngineTemperatureSensor {};
```

### Functions & Methods
```cpp
void updateSensorValue() {};     // camelCase
double calculateAverage() {};
```

### Variables
```cpp
int sensorCount;                 // camelCase for local/public
int sensor_count_;               // snake_case with _ for private members
const int MAX_SENSORS = 10;      // UPPER_CASE for constants
```

### Files
```cpp
Sensor.hpp                       // PascalCase matching class name
Utils.hpp                        // Helper utilities
```

---

## Header Guards

Use `#pragma once`:
```cpp
#pragma once

namespace VehicleSystem {
    // code
}
```

---

## Documentation

Use Doxygen-style comments:
```cpp
/**
 * @brief Updates the sensor value
 * @param newValue The new sensor reading
 * @return true if update successful, false otherwise
 */
bool updateValue(double newValue);
```

---

## Formatting

- **Indentation**: 4 spaces (no tabs)
- **Line length**: 100 characters max
- **Braces**: Same line for functions, classes
```cpp
class Sensor {
public:
    void update() {
        // code
    }
};
```

---

## Modern C++ Practices

### Smart Pointers
```cpp
// Prefer unique_ptr for exclusive ownership
std::unique_ptr<Sensor> sensor = std::make_unique<EngineSensor>();

// Use shared_ptr only when shared ownership needed
std::shared_ptr<Alert> alert = std::make_shared<Alert>();
```

### RAII
```cpp
// File handles auto-close
{
    std::ofstream file("log.txt");
    file << "data";
} // file automatically closed
```

### Range-based loops
```cpp
for (const auto& sensor : sensors) {
    sensor->update();
}
```

---

## Thread Safety

### Mutex Protection
```cpp
std::mutex mtx;

void addAlert(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mtx);
    alerts_.push_back(alert);
}
```

---

## Git Commit Messages

Format:
[Module] Brief description
Detailed explanation if needed

Examples:
[Sensor] Add EngineTemperatureSensor implementation
[Alert] Fix race condition in AlertManager::addAlert()
[Threading] Implement watchdog heartbeat mechanism
[Config] Add JSON parsing for driver profiles

---

## Code Review Checklist

- [ ] No memory leaks (use smart pointers)
- [ ] Thread-safe (mutexes where needed)
- [ ] Exception-safe (RAII, try-catch)
- [ ] Documented (Doxygen comments)
- [ ] Formatted (clang-format)
- [ ] Tested (unit tests pass)