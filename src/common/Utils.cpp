#include "common/Utils.hpp"

namespace VehicleSystem {
namespace Utils {

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    return timestampToString(now);
}

int32_t getRandomInt(int32_t min, int32_t max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dis(min, max);
    return dis(gen);
}

double getRandomDouble(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleepMs(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

std::string timestampToString(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace Utils
} // namespace VehicleSystem