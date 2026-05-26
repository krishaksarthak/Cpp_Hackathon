#include "Logger.hpp"
#include <iostream>
#include <ctime>

Logger::Logger(const std::string& filename) {
    file.open(filename, std::ios::app);
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    std::time_t now = std::time(nullptr);
    file << std::ctime(&now) << ": " << message << "\n";
}