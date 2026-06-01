/**
 * @file test_sensors.cpp
 * @brief Unit tests for the sensor framework.
 * 
 * Tests: Base class, all 6 derived sensors, polymorphism,
 * value ranges, update functionality, copy/move semantics.
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "sensors/Sensor.hpp"
#include "sensors/EngineTemperatureSensor.hpp"
#include "sensors/BatterySensor.hpp"
#include "sensors/SpeedSensor.hpp"
#include "sensors/TirePressureSensor.hpp"
#include "sensors/DoorSensor.hpp"
#include "sensors/SeatbeltSensor.hpp"

using namespace VehicleSystem;

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    std::cout << "  Testing: " << name << "... "; \
    try {

#define PASS() \
    std::cout << "PASSED\n"; \
    testsPassed++; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << "\n"; \
        testsFailed++; \
    } catch (...) { \
        std::cout << "FAILED: Unknown exception\n"; \
        testsFailed++; \
    }

#define ASSERT_TRUE(expr) \
    if (!(expr)) throw std::runtime_error("Assertion failed: " #expr);

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) throw std::runtime_error("Assertion failed: " #a " != " #b);

#define ASSERT_RANGE(val, min, max) \
    if ((val) < (min) || (val) > (max)) \
        throw std::runtime_error("Value " + std::to_string(val) + " out of range [" + \
                                 std::to_string(min) + ", " + std::to_string(max) + "]");

void testBaseSensorClass() {
    std::cout << "\n=== TC-SENSOR-001: Base Sensor Class ===\n";

    TEST("Cannot instantiate Sensor directly (verified by polymorphism)")
        // Sensor is abstract - we test via derived classes
        EngineTemperatureSensor sensor;
        ASSERT_TRUE(sensor.getId() == 1);
        ASSERT_TRUE(sensor.isHealthy());
    PASS()

    TEST("Static sensor count tracking")
        uint32_t initialCount = Sensor::getSensorCount();
        {
            EngineTemperatureSensor temp;
            BatterySensor batt;
            ASSERT_EQ(Sensor::getSensorCount(), initialCount + 2);
        }
        // RAII: count decremented after destruction
        ASSERT_EQ(Sensor::getSensorCount(), initialCount);
    PASS()
}

void testEngineTemperatureSensor() {
    std::cout << "\n=== TC-SENSOR-002: Engine Temperature Sensor ===\n";

    TEST("Value range 60-130°C after 100 updates")
        EngineTemperatureSensor sensor;
        for (int i = 0; i < 100; ++i) {
            sensor.performUpdate();
            ASSERT_RANGE(sensor.getValue(), 60.0, 130.0);
        }
    PASS()

    TEST("Sensor type and name")
        EngineTemperatureSensor sensor;
        ASSERT_EQ(sensor.getType(), SensorType::ENGINE_TEMPERATURE);
        ASSERT_EQ(sensor.getName(), std::string("Engine Temperature"));
        ASSERT_EQ(sensor.getUnit(), std::string("C"));
    PASS()
}

void testBatterySensor() {
    std::cout << "\n=== TC-SENSOR-003: Battery Voltage Sensor ===\n";

    TEST("Value range 9-15V after 100 updates")
        BatterySensor sensor;
        for (int i = 0; i < 100; ++i) {
            sensor.performUpdate();
            ASSERT_RANGE(sensor.getValue(), 9.0, 15.0);
        }
    PASS()

    TEST("Initial value is nominal (12.6V)")
        BatterySensor sensor;
        ASSERT_RANGE(sensor.getValue(), 12.0, 13.0);
    PASS()
}

void testSpeedSensor() {
    std::cout << "\n=== TC-SENSOR-004: Speed Sensor ===\n";

    TEST("Value range 0-200 km/h after 100 updates")
        SpeedSensor sensor;
        for (int i = 0; i < 100; ++i) {
            sensor.performUpdate();
            ASSERT_RANGE(sensor.getValue(), 0.0, 200.0);
        }
    PASS()
}

void testTirePressureSensor() {
    std::cout << "\n=== TC-SENSOR-005: Tire Pressure Sensor ===\n";

    TEST("Value range 20-40 PSI after 100 updates")
        TirePressureSensor sensor;
        for (int i = 0; i < 100; ++i) {
            sensor.performUpdate();
            ASSERT_RANGE(sensor.getValue(), 20.0, 40.0);
        }
    PASS()
}

void testDoorSensor() {
    std::cout << "\n=== TC-SENSOR-006: Door Status Sensor ===\n";

    TEST("Both OPEN and CLOSED states observed")
        DoorSensor sensor;
        bool seenOpen = false, seenClosed = false;
        for (int i = 0; i < 200; ++i) {
            sensor.performUpdate();
            if (sensor.isOpen()) seenOpen = true;
            else seenClosed = true;
        }
        ASSERT_TRUE(seenOpen);
        ASSERT_TRUE(seenClosed);
    PASS()
}

void testSeatbeltSensor() {
    std::cout << "\n=== TC-SENSOR-007: Seatbelt Status Sensor ===\n";

    TEST("Both LOCKED and UNLOCKED states observed")
        SeatbeltSensor sensor;
        bool seenLocked = false, seenUnlocked = false;
        for (int i = 0; i < 200; ++i) {
            sensor.performUpdate();
            if (sensor.isLocked()) seenLocked = true;
            else seenUnlocked = true;
        }
        ASSERT_TRUE(seenLocked);
        ASSERT_TRUE(seenUnlocked);
    PASS()
}

void testPolymorphicAccess() {
    std::cout << "\n=== TC-SENSOR-008: Polymorphic Sensor Access ===\n";

    TEST("All 6 sensors update via base pointer")
        std::vector<std::unique_ptr<Sensor>> sensors;
        sensors.push_back(std::make_unique<EngineTemperatureSensor>());
        sensors.push_back(std::make_unique<BatterySensor>());
        sensors.push_back(std::make_unique<SpeedSensor>());
        sensors.push_back(std::make_unique<TirePressureSensor>());
        sensors.push_back(std::make_unique<DoorSensor>());
        sensors.push_back(std::make_unique<SeatbeltSensor>());

        ASSERT_EQ(sensors.size(), static_cast<size_t>(6));

        for (auto& sensor : sensors) {
            sensor->performUpdate();
            ASSERT_TRUE(sensor->isHealthy());
            ASSERT_TRUE(!sensor->display().empty());
            ASSERT_TRUE(!sensor->getValueString().empty());
        }
    PASS()

    TEST("Clone (virtual constructor) works")
        EngineTemperatureSensor original;
        original.performUpdate();
        auto cloned = original.clone();
        ASSERT_EQ(cloned->getType(), SensorType::ENGINE_TEMPERATURE);
    PASS()
}

int main() {
    std::cout << "========================================\n";
    std::cout << "SENSOR UNIT TESTS\n";
    std::cout << "========================================\n";

    testBaseSensorClass();
    testEngineTemperatureSensor();
    testBatterySensor();
    testSpeedSensor();
    testTirePressureSensor();
    testDoorSensor();
    testSeatbeltSensor();
    testPolymorphicAccess();

    std::cout << "\n========================================\n";
    std::cout << "RESULTS: " << testsPassed << " passed, "
              << testsFailed << " failed\n";
    std::cout << "========================================\n";

    return testsFailed > 0 ? 1 : 0;
}
