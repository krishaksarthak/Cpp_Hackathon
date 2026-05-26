/**
 * @file test_threading.cpp
 * @brief Unit tests for threading, watchdog, config, logger, and statistics.
 * 
 * Tests: Thread safety, mutex protection, watchdog health,
 * config loading, driver profiles, event logger, statistics.
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdio>

#include "common/Types.hpp"
#include "common/Utils.hpp"
#include "threading/Watchdog.hpp"
#include "dashboard/VehicleStatistics.hpp"
#include "logging/EventLogger.hpp"
#include "config/ConfigManager.hpp"
#include "config/DriverProfile.hpp"
#include "alerts/Alert.hpp"

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

void testWatchdog() {
    std::cout << "\n=== TC-THREAD-008: Watchdog ===\n";

    TEST("Register and heartbeat")
        Watchdog wd(1000);
        wd.registerThread("TestThread1");
        wd.registerThread("TestThread2");
        wd.heartbeat("TestThread1");
        wd.heartbeat("TestThread2");
        ASSERT_TRUE(wd.checkHealth());
        ASSERT_EQ(wd.getRegisteredThreadCount(), static_cast<size_t>(2));
    PASS()

    TEST("Detect unhealthy thread (timeout)")
        Watchdog wd(100); // 100ms timeout
        wd.registerThread("FastThread");
        wd.registerThread("SlowThread");
        wd.heartbeat("FastThread");
        wd.heartbeat("SlowThread");
        
        // Wait for timeout
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Send heartbeat for only one
        wd.heartbeat("FastThread");
        
        ASSERT_TRUE(!wd.checkHealth()); // SlowThread should be unhealthy
        auto unhealthy = wd.getUnhealthyThreads();
        ASSERT_TRUE(!unhealthy.empty());
    PASS()

    TEST("Health report generation")
        Watchdog wd(5000);
        wd.registerThread("Thread1");
        wd.heartbeat("Thread1");
        std::string report = wd.getHealthReport();
        ASSERT_TRUE(!report.empty());
        ASSERT_TRUE(report.find("Thread1") != std::string::npos);
    PASS()

    TEST("Uptime tracking")
        Watchdog wd;
        std::string uptime = wd.getUptime();
        ASSERT_TRUE(!uptime.empty());
        ASSERT_TRUE(uptime.find(":") != std::string::npos);
    PASS()
}

void testVehicleStatistics() {
    std::cout << "\n=== TC-DASH-003: Vehicle Statistics ===\n";

    TEST("Speed recording and average")
        VehicleStatistics stats;
        stats.recordSpeed(60.0);
        stats.recordSpeed(80.0);
        stats.recordSpeed(100.0);
        double avg = stats.getAverageSpeed();
        ASSERT_TRUE(avg > 75.0 && avg < 85.0); // ~80
    PASS()

    TEST("Peak temperature tracking")
        VehicleStatistics stats;
        stats.recordTemperature(80.0);
        stats.recordTemperature(95.0);
        stats.recordTemperature(88.0);
        ASSERT_TRUE(stats.getPeakTemperature() >= 95.0);
    PASS()

    TEST("Alert frequency tracking")
        VehicleStatistics stats;
        stats.recordAlert(AlertSeverity::CRITICAL, "ENGINE_OVERHEAT");
        stats.recordAlert(AlertSeverity::CRITICAL, "ENGINE_OVERHEAT");
        stats.recordAlert(AlertSeverity::WARNING, "LOW_BATTERY");
        ASSERT_EQ(stats.getTotalAlerts(), static_cast<size_t>(3));
        auto freq = stats.getAlertFrequency();
        ASSERT_EQ(freq["ENGINE_OVERHEAT"], 2);
    PASS()

    TEST("Most frequent alert (uses STL max_element + lambda)")
        VehicleStatistics stats;
        stats.recordAlert(AlertSeverity::WARNING, "OVERSPEED");
        stats.recordAlert(AlertSeverity::WARNING, "OVERSPEED");
        stats.recordAlert(AlertSeverity::WARNING, "OVERSPEED");
        stats.recordAlert(AlertSeverity::CRITICAL, "ENGINE_OVERHEAT");
        std::string most = stats.getMostFrequentAlert();
        ASSERT_TRUE(most.find("OVERSPEED") != std::string::npos);
    PASS()

    TEST("Formatted statistics output")
        VehicleStatistics stats;
        stats.recordSpeed(100.0);
        stats.recordTemperature(90.0);
        std::string formatted = stats.getFormattedStats();
        ASSERT_TRUE(!formatted.empty());
        ASSERT_TRUE(formatted.find("Avg Speed") != std::string::npos);
    PASS()

    TEST("Reset clears all data")
        VehicleStatistics stats;
        stats.recordSpeed(100);
        stats.recordAlert(AlertSeverity::INFO, "test");
        stats.reset();
        ASSERT_TRUE(stats.getAverageSpeed() < 0.1);
        ASSERT_EQ(stats.getTotalAlerts(), static_cast<size_t>(0));
    PASS()
}

void testEventLogger() {
    std::cout << "\n=== TC-LOG-001 to 005: Event Logger ===\n";

    std::string testLogPath = "test_log_output.log";
    // Clean up any previous test file
    std::remove(testLogPath.c_str());

    TEST("RAII - log file created on construction")
        {
            EventLogger logger(testLogPath);
            // File should exist
            std::ifstream check(testLogPath);
            ASSERT_TRUE(check.good());
        }
        // After destruction, file should not be locked
        std::ifstream check(testLogPath);
        ASSERT_TRUE(check.good());
    PASS()

    TEST("Log events and process queue")
        {
            EventLogger logger(testLogPath);
            logger.logEvent(AlertSeverity::INFO, "Test event 1", "TEST");
            logger.logEvent(AlertSeverity::WARNING, "Test event 2", "TEST");
            logger.logEvent(AlertSeverity::CRITICAL, "Test event 3", "TEST");
            logger.processQueue();
        }
        // Check file has content
        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        ASSERT_TRUE(content.find("Test event 1") != std::string::npos);
    PASS()

    TEST("Lambda-based event search")
        EventLogger logger(testLogPath);
        logger.logEvent(AlertSeverity::INFO, "Info message", "SRC1");
        logger.logEvent(AlertSeverity::CRITICAL, "Critical message", "SRC2");
        logger.logEvent(AlertSeverity::WARNING, "Warning message", "SRC3");
        logger.processQueue();

        // Search for CRITICAL events using lambda
        auto criticals = logger.searchEvents(
            [](const LogEntry& e) {
                return e.severity == AlertSeverity::CRITICAL;
            });
        ASSERT_TRUE(!criticals.empty());
        for (const auto& e : criticals) {
            ASSERT_EQ(e.severity, AlertSeverity::CRITICAL);
        }
    PASS()

    TEST("Log count tracking")
        EventLogger logger(testLogPath);
        logger.logEvent(AlertSeverity::INFO, "A", "TEST");
        logger.logEvent(AlertSeverity::INFO, "B", "TEST");
        logger.processQueue();
        ASSERT_TRUE(logger.getTotalLogCount() >= 2);
    PASS()

    // Clean up
    std::remove(testLogPath.c_str());
}

void testConfigManager() {
    std::cout << "\n=== TC-CONFIG-001 to 004: Configuration ===\n";

    TEST("Load valid config.json")
        auto& config = ConfigManager::getInstance();
        // Try loading from various paths
        bool loaded = config.loadConfig("data/config.json") ||
                      config.loadConfig("../data/config.json") ||
                      config.loadConfig("../../data/config.json");
        if (loaded) {
            int tempCritical = config.getInt("sensors", "engine_temp_threshold_critical", 110);
            ASSERT_TRUE(tempCritical > 0);
            std::cout << "(temp_critical=" << tempCritical << ") ";
        } else {
            std::cout << "(config not found, testing defaults) ";
            int defaultVal = config.getInt("sensors", "engine_temp_threshold_critical", 110);
            ASSERT_EQ(defaultVal, 110);
        }
    PASS()

    TEST("Default values for missing keys")
        auto& config = ConfigManager::getInstance();
        int missing = config.getInt("nonexistent", "key", 42);
        ASSERT_EQ(missing, 42);
        std::string missingStr = config.getString("nonexistent", "key", "default");
        ASSERT_EQ(missingStr, std::string("default"));
    PASS()
}

void testDriverProfile() {
    std::cout << "\n=== TC-CONFIG-003: Driver Profiles ===\n";

    TEST("Default profile values")
        DriverProfile profile;
        ASSERT_EQ(profile.getSpeedLimit(), 120);
        ASSERT_EQ(profile.getEngineTempCritical(), 110);
    PASS()

    TEST("Load eco_mode profile")
        DriverProfile profile;
        bool loaded = profile.loadProfile("data/driver_profiles/eco_mode.json") ||
                      profile.loadProfile("../data/driver_profiles/eco_mode.json") ||
                      profile.loadProfile("../../data/driver_profiles/eco_mode.json");
        if (loaded) {
            ASSERT_EQ(profile.getSpeedLimit(), 100);
            ASSERT_EQ(profile.getName(), std::string("Eco Mode"));
            std::cout << "(eco loaded) ";
        } else {
            std::cout << "(eco not found, skipped) ";
        }
    PASS()

    TEST("Load sport_mode profile")
        DriverProfile profile;
        bool loaded = profile.loadProfile("data/driver_profiles/sport_mode.json") ||
                      profile.loadProfile("../data/driver_profiles/sport_mode.json") ||
                      profile.loadProfile("../../data/driver_profiles/sport_mode.json");
        if (loaded) {
            ASSERT_EQ(profile.getSpeedLimit(), 180);
            ASSERT_EQ(profile.getName(), std::string("Sport Mode"));
            std::cout << "(sport loaded) ";
        } else {
            std::cout << "(sport not found, skipped) ";
        }
    PASS()

    TEST("Available profiles list")
        auto profiles = DriverProfile::getAvailableProfiles();
        ASSERT_EQ(profiles.size(), static_cast<size_t>(3));
    PASS()
}

void testThreadSafety() {
    std::cout << "\n=== TC-THREAD-006: Thread Safety (Mutex) ===\n";

    TEST("Concurrent watchdog heartbeats")
        Watchdog wd(5000);
        wd.registerThread("T1");
        wd.registerThread("T2");
        wd.registerThread("T3");

        // Launch threads that heartbeat concurrently
        std::vector<std::thread> threads;
        for (int i = 1; i <= 3; ++i) {
            threads.emplace_back([&wd, i]() {
                std::string name = "T" + std::to_string(i);
                for (int j = 0; j < 100; ++j) {
                    wd.heartbeat(name);
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            });
        }
        for (auto& t : threads) t.join();
        ASSERT_TRUE(wd.checkHealth());
    PASS()

    TEST("Concurrent statistics recording")
        VehicleStatistics stats;
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([&stats]() {
                for (int j = 0; j < 100; ++j) {
                    stats.recordSpeed(60.0 + j);
                    stats.recordTemperature(80.0 + j * 0.1);
                    stats.recordAlert(AlertSeverity::INFO, "test");
                }
            });
        }
        for (auto& t : threads) t.join();
        ASSERT_TRUE(stats.getTotalAlerts() == 400);
        ASSERT_TRUE(stats.getAverageSpeed() > 0);
    PASS()
}

int main() {
    std::cout << "========================================\n";
    std::cout << "THREADING, CONFIG & INTEGRATION TESTS\n";
    std::cout << "========================================\n";

    testWatchdog();
    testVehicleStatistics();
    testEventLogger();
    testConfigManager();
    testDriverProfile();
    testThreadSafety();

    std::cout << "\n========================================\n";
    std::cout << "RESULTS: " << testsPassed << " passed, "
              << testsFailed << " failed\n";
    std::cout << "========================================\n";

    return testsFailed > 0 ? 1 : 0;
}
