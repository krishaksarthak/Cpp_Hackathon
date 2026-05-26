#pragma once
#include <fstream>
#include <mutex>
#include <string>

class Logger {
    std::ofstream file;
    std::mutex mtx;

public:
    Logger(const std::string& filename);
    void log(const std::string& message);
};