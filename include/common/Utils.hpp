#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

namespace VehicleSystem {
namespace Utils {

/**
 * @brief Get current timestamp as formatted string
 * @return Formatted timestamp string
 */
std::string getCurrentTimestamp();

/**
 * @brief Generate random integer in range [min, max]
 */
int getRandomInt(int min, int max);

/**
 * @brief Generate random double in range [min, max]
 */
double getRandomDouble(double min, double max);

/**
 * @brief Sleep for specified milliseconds
 */
void sleepMs(int milliseconds);

/**
 * @brief Convert timestamp to string
 */
std::string timestampToString(const std::chrono::system_clock::time_point& tp);

} // namespace Utils
} // namespace VehicleSystem