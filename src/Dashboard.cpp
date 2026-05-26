#include "Dashboard.hpp"
#include <iostream>
#include <functional>

void Dashboard::addSensor(std::shared_ptr<Sensor> sensor) {
    sensors.push_back(sensor);
}

void Dashboard::updateSensors() {
    for (auto& s : sensors) {
        s->update();
    }
}

void Dashboard::checkAlerts() {
    alerts.clear();

    for (auto& s : sensors) {

        if (s->getName() == "Engine" && s->getValue() > 110)
            alerts.emplace_back("Engine Overheat", Severity::CRITICAL);

        if (s->getName() == "Battery" && s->getValue() < 10)
            alerts.emplace_back("Low Battery", Severity::WARNING);

        if (s->getName() == "Speed" && s->getValue() > 120)
            alerts.emplace_back("Overspeed", Severity::WARNING);
    }

    // ✅ Optional: store history if you declared it
    history.insert(history.end(), alerts.begin(), alerts.end());
}

void Dashboard::display() {
    std::cout << "\n---- DASHBOARD ----\n";

    for (auto& s : sensors) {
        s->display();
    }

    std::cout << "\nAlerts:\n";

    if (alerts.empty()) {
        std::cout << "No active alerts\n";
    } else {
        for (auto& a : alerts) {
            std::cout << a << "\n";
        }
    }
}

void Dashboard::searchAlerts(std::function<bool(const Alert&)> filter) {
    std::cout << "\nFiltered Alerts:\n";

    if (history.empty()) {
        std::cout << "No alert history available\n";
        return;
    }

    for (const auto& a : history) {
        if (filter(a)) {
            std::cout << a << "\n";
        }
    }
}
