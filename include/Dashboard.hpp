#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "Sensor.hpp"
#include "Alert.hpp"

class Dashboard {
    std::vector<std::shared_ptr<Sensor>> sensors;
    std::vector<Alert> alerts;
    std::vector<Alert> history;   // ✅ FIX: ADD THIS

public:
    void addSensor(std::shared_ptr<Sensor> sensor);
    void updateSensors();
    void checkAlerts();
    void display();

    // ✅ FIX: correct function signature
    void searchAlerts(std::function<bool(const Alert&)> filter);
};