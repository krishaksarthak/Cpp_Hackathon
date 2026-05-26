#include "Sensor.hpp"
#include <iostream>
#include <random>

static double randomRange(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

void EngineTempSensor::update() {
    value = randomRange(80, 125);
}
void EngineTempSensor::display() const {
    std::cout << "Engine Temp: " << value << " C\n";
}
std::string EngineTempSensor::getName() const { return "Engine"; }
double EngineTempSensor::getValue() const { return value; }


void BatterySensor::update() {
    value = randomRange(9, 13);
}
void BatterySensor::display() const {
    std::cout << "Battery: " << value << " V\n";
}
std::string BatterySensor::getName() const { return "Battery"; }
double BatterySensor::getValue() const { return value; }


void SpeedSensor::update() {
    value = randomRange(0, 150);
}
void SpeedSensor::display() const {
    std::cout << "Speed: " << value << " km/h\n";
}
std::string SpeedSensor::getName() const { return "Speed"; }
double SpeedSensor::getValue() const { return value; }