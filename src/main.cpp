#include "Dashboard.hpp"
#include "Logger.hpp"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <functional>

int main() {
    Dashboard dashboard;
    Logger logger("logs/vehicle_log.txt");

    dashboard.addSensor(std::make_shared<EngineTempSensor>());
    dashboard.addSensor(std::make_shared<BatterySensor>());
    dashboard.addSensor(std::make_shared<SpeedSensor>());

    std::mutex mtx;
    std::atomic<bool> running(true);

    // ✅ SENSOR THREAD
    std::thread sensorThread([&]() {
        while (running) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                dashboard.updateSensors();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // ✅ MONITOR THREAD
    std::thread monitorThread([&]() {
        while (running) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                dashboard.checkAlerts();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // ✅ SINGLE USER LOOP (NO INPUT THREAD NEEDED)
    while (running) {
        int choice;

        std::cout << "\n===== MENU =====\n";
        std::cout << "1. Show Dashboard\n";
        std::cout << "2. Search Critical Alerts\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter choice: ";

        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);

        if (choice == 1) {
            dashboard.display();
        }
        else if (choice == 2) {
            std::cout << "\nCritical Alerts:\n";

            dashboard.searchAlerts([](const Alert& a) {
                return a.getSeverity() == Severity::CRITICAL;
            });
        }
        else if (choice == 0) {
            std::cout << "Shutting down system...\n";
            running = false;
        }
    }

    sensorThread.join();
    monitorThread.join();

    std::cout << "System terminated safely\n";
    return 0;
}
