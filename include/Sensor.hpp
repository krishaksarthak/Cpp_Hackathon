#pragma once
#include <string>
#include <memory>

class Sensor {
public:
    virtual void update() = 0;
    virtual void display() const = 0;
    virtual std::string getName() const = 0;
    virtual double getValue() const = 0;
    virtual ~Sensor() = default;
};

class EngineTempSensor : public Sensor {
    double value;
public:
    void update() override;
    void display() const override;
    std::string getName() const override;
    double getValue() const override;
};

class BatterySensor : public Sensor {
    double value;
public:
    void update() override;
    void display() const override;
    std::string getName() const override;
    double getValue() const override;
};

class SpeedSensor : public Sensor {
    double value;
public:
    void update() override;
    void display() const override;
    std::string getName() const override;
    double getValue() const override;
};
