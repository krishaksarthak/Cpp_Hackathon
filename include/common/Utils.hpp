#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <cstdint>

namespace VehicleSystem {
namespace Utils {

/**
 * @brief Get current timestamp as formatted string
 * @return Formatted timestamp string
 */
std::string getCurrentTimestamp();

/**
 * @brief Generate random integer in range [min, max]
 * @param min The minimum value (inclusive)
 * @param max The maximum value (inclusive)
 * @return int32_t The randomly generated integer
 */
int32_t getRandomInt(int32_t min, int32_t max);

/**
 * @brief Generate random double in range [min, max]
 * @param min The minimum value (inclusive)
 * @param max The maximum value (inclusive)
 * @return double The randomly generated double
 */
double getRandomDouble(double min, double max);

/**
 * @brief Sleep for specified milliseconds
 * @param milliseconds The duration to sleep in milliseconds
 */
void sleepMs(uint32_t milliseconds);

/**
 * @brief Convert timestamp to string
 * @param tp The time point to convert
 * @return std::string The formatted timestamp string
 */
std::string timestampToString(const std::chrono::system_clock::time_point& tp);

} // namespace Utils
} // namespace VehicleSystem