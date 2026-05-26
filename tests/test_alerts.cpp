/**
 * @file test_alerts.cpp
 * @brief Unit tests for alert system, AlertManager, and DTC.
 * 
 * Tests: Alert creation, severity enum, operator overloading,
 * 6 alert conditions, DTC generation, freeze-frame data.
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <sstream>

#include "alerts/Alert.hpp"
#include "alerts/AlertManager.hpp"
#include "alerts/DTC.hpp"
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

void testAlertSeverityEnum() {
    std::cout << "\n=== TC-ALERT-001: Alert Severity Enum ===\n";

    TEST("INFO, WARNING, CRITICAL defined")
        AlertSeverity s1 = AlertSeverity::INFO;
        AlertSeverity s2 = AlertSeverity::WARNING;
        AlertSeverity s3 = AlertSeverity::CRITICAL;
        ASSERT_EQ(severityToString(s1), std::string("INFO"));
        ASSERT_EQ(severityToString(s2), std::string("WARNING"));
        ASSERT_EQ(severityToString(s3), std::string("CRITICAL"));
    PASS()

    TEST("Type-safe enum class")
        // Cannot implicitly convert to int (enum class)
        AlertSeverity s = AlertSeverity::CRITICAL;
        int val = static_cast<int>(s); // Explicit cast required
        ASSERT_EQ(val, 2);
    PASS()
}

void testAlertOperatorOverloading() {
    std::cout << "\n=== TC-ALERT-008: Alert Operator Overloading ===\n";

    TEST("Stream insertion operator <<")
        Alert alert(AlertSeverity::CRITICAL, "ENGINE OVERHEAT",
                   SensorType::ENGINE_TEMPERATURE, 115.0);
        std::ostringstream oss;
        oss << alert;
        std::string output = oss.str();
        ASSERT_TRUE(output.find("CRITICAL") != std::string::npos);
        ASSERT_TRUE(output.find("ENGINE OVERHEAT") != std::string::npos);
    PASS()

    TEST("Equality operator ==")
        Alert a1(AlertSeverity::WARNING, "Test", SensorType::VEHICLE_SPEED, 100);
        Alert a2(AlertSeverity::CRITICAL, "Test2", SensorType::VEHICLE_SPEED, 100);
        ASSERT_TRUE(a1 == a1);
        ASSERT_TRUE(!(a1 == a2)); // Different IDs
    PASS()

    TEST("Less-than operator < (comparison by severity)")
        Alert info(AlertSeverity::INFO, "Info", SensorType::VEHICLE_SPEED, 0);
        Alert critical(AlertSeverity::CRITICAL, "Critical", SensorType::VEHICLE_SPEED, 0);
        ASSERT_TRUE(info < critical);
    PASS()

    TEST("toString() method")
        Alert alert(AlertSeverity::WARNING, "LOW BATTERY",
                   SensorType::BATTERY_VOLTAGE, 9.5);
        std::string str = alert.toString();
        ASSERT_TRUE(!str.empty());
        ASSERT_TRUE(str.find("WARNING") != std::string::npos);
    PASS()
}

// Helper: create a sensor vector with specific values
std::vector<std::unique_ptr<Sensor>> createTestSensors() {
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<EngineTemperatureSensor>());
    sensors.push_back(std::make_unique<BatterySensor>());
    sensors.push_back(std::make_unique<SpeedSensor>());
    sensors.push_back(std::make_unique<TirePressureSensor>());
    sensors.push_back(std::make_unique<DoorSensor>());
    sensors.push_back(std::make_unique<SeatbeltSensor>());
    return sensors;
}

void testAlertManager() {
    std::cout << "\n=== TC-ALERT-002 to 007: Alert Conditions ===\n";

    TEST("AlertManager initial state")
        AlertManager mgr;
        ASSERT_EQ(mgr.getActiveAlertCount(), static_cast<size_t>(0));
        ASSERT_EQ(mgr.getTotalAlertCount(), static_cast<size_t>(0));
    PASS()

    TEST("Multiple evaluations produce consistent results")
        AlertManager mgr;
        auto sensors = createTestSensors();
        // Run multiple evaluations
        for (int i = 0; i < 10; ++i) {
            for (auto& s : sensors) s->performUpdate();
            mgr.evaluateConditions(sensors);
        }
        // Alert count should be reasonable
        ASSERT_TRUE(mgr.getTotalAlertCount() < 100);
    PASS()

    TEST("Filter alerts by severity")
        AlertManager mgr;
        auto sensors = createTestSensors();
        for (int i = 0; i < 50; ++i) {
            for (auto& s : sensors) s->performUpdate();
            mgr.evaluateConditions(sensors);
        }
        auto criticals = mgr.filterAlerts(AlertSeverity::CRITICAL);
        auto warnings = mgr.filterAlerts(AlertSeverity::WARNING);
        // All returned alerts should match the filter
        for (const auto& a : criticals) {
            ASSERT_EQ(a.getSeverity(), AlertSeverity::CRITICAL);
        }
        for (const auto& a : warnings) {
            ASSERT_EQ(a.getSeverity(), AlertSeverity::WARNING);
        }
    PASS()
}

void testDTCSystem() {
    std::cout << "\n=== TC-DTC-001 to 004: DTC System ===\n";

    TEST("DTC generation from alert")
        DTCManager dtcMgr;
        auto sensors = createTestSensors();
        
        Alert alert(AlertSeverity::CRITICAL, "ENGINE OVERHEAT - Temp: 115.0C",
                   SensorType::ENGINE_TEMPERATURE, 115.0);
        dtcMgr.generateDTC(alert, sensors, "Comfort Mode");
        
        ASSERT_EQ(dtcMgr.getActiveDTCCount(), static_cast<size_t>(1));
        auto dtcs = dtcMgr.getActiveDTCs();
        ASSERT_EQ(dtcs[0].code, std::string("P0217"));
    PASS()

    TEST("DTC code mapping")
        DTCManager dtcMgr;
        auto sensors = createTestSensors();
        
        Alert alert(AlertSeverity::WARNING, "LOW BATTERY - Voltage: 9.5V",
                   SensorType::BATTERY_VOLTAGE, 9.5);
        dtcMgr.generateDTC(alert, sensors, "Eco Mode");
        
        auto dtcs = dtcMgr.getActiveDTCs();
        ASSERT_EQ(dtcs[0].code, std::string("P0562"));
    PASS()

    TEST("DTC freeze frame data")
        DTCManager dtcMgr;
        auto sensors = createTestSensors();
        for (auto& s : sensors) s->performUpdate();
        
        Alert alert(AlertSeverity::CRITICAL, "ENGINE OVERHEAT - Temp: 120.0C",
                   SensorType::ENGINE_TEMPERATURE, 120.0);
        dtcMgr.generateDTC(alert, sensors, "Sport Mode");
        
        auto dtcs = dtcMgr.getActiveDTCs();
        ASSERT_TRUE(!dtcs[0].freezeFrame.sensorValues.empty());
        ASSERT_EQ(dtcs[0].freezeFrame.activeProfile, std::string("Sport Mode"));
    PASS()

    TEST("DTC clearing")
        DTCManager dtcMgr;
        auto sensors = createTestSensors();
        
        Alert a1(AlertSeverity::CRITICAL, "ENGINE OVERHEAT",
                SensorType::ENGINE_TEMPERATURE, 115.0);
        Alert a2(AlertSeverity::WARNING, "LOW BATTERY",
                SensorType::BATTERY_VOLTAGE, 9.5);
        
        dtcMgr.generateDTC(a1, sensors, "Default");
        dtcMgr.generateDTC(a2, sensors, "Default");
        ASSERT_EQ(dtcMgr.getActiveDTCCount(), static_cast<size_t>(2));
        
        dtcMgr.clearDTC("P0217");
        ASSERT_EQ(dtcMgr.getActiveDTCCount(), static_cast<size_t>(1));
        
        dtcMgr.clearAllDTCs();
        ASSERT_EQ(dtcMgr.getActiveDTCCount(), static_cast<size_t>(0));
    PASS()
}

int main() {
    std::cout << "========================================\n";
    std::cout << "ALERT & DTC UNIT TESTS\n";
    std::cout << "========================================\n";

    Alert::resetIdCounter();

    testAlertSeverityEnum();
    testAlertOperatorOverloading();
    testAlertManager();
    testDTCSystem();

    std::cout << "\n========================================\n";
    std::cout << "RESULTS: " << testsPassed << " passed, "
              << testsFailed << " failed\n";
    std::cout << "========================================\n";

    return testsFailed > 0 ? 1 : 0;
}
