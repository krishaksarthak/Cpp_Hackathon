#pragma once

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <functional>
#include <string>
#include <sstream>

namespace VehicleSystem {

/**
 * @brief Generic circular buffer for storing time-series sensor data.
 *
 * Demonstrates: Class Templates, template type parameters,
 * STL algorithms (accumulate, max_element, copy_if),
 * iterator support, lambda-based filtering.
 *
 * Maps to: Generic data buffer in AUTOSAR Adaptive Platform.
 * Used by VehicleStatistics to hold speed/temperature histories.
 *
 * @tparam T The data type to store (e.g., double, int, float)
 */
template <typename T>
class SensorDataBuffer {
public:
    /**
     * @brief Construct a buffer with a fixed maximum capacity.
     * @param capacity Max number of elements. Oldest data is overwritten when full.
     */
    explicit SensorDataBuffer(size_t capacity = 100)
        : m_capacity(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("SensorDataBuffer capacity must be > 0");
        }
        m_data.reserve(capacity);
    }

    // --- Rule of Five: Copy/Move semantics explicitly defined ---
    SensorDataBuffer(const SensorDataBuffer&) = default;
    SensorDataBuffer& operator=(const SensorDataBuffer&) = default;
    SensorDataBuffer(SensorDataBuffer&&) noexcept = default;
    SensorDataBuffer& operator=(SensorDataBuffer&&) noexcept = default;
    ~SensorDataBuffer() = default;

    /**
     * @brief Push a new value into the buffer.
     * If the buffer is at capacity, the oldest value is overwritten.
     */
    void push(const T& value) {
        if (m_data.size() >= m_capacity) {
            m_data.erase(m_data.begin()); // Remove oldest (FIFO)
        }
        m_data.push_back(value);
    }

    /**
     * @brief Push via move semantics
     */
    void push(T&& value) {
        if (m_data.size() >= m_capacity) {
            m_data.erase(m_data.begin());
        }
        m_data.push_back(std::move(value));
    }

    /**
     * @brief Compute the arithmetic mean of all stored values.
     * @return Average value, or 0 if empty.
     */
    T average() const {
        if (m_data.empty()) return T{};
        T sum = std::accumulate(m_data.begin(), m_data.end(), T{});
        return sum / static_cast<T>(m_data.size());
    }

    /**
     * @brief Get the maximum stored value.
     * @return Max value, or T{} if empty.
     */
    T maximum() const {
        if (m_data.empty()) return T{};
        return *std::max_element(m_data.begin(), m_data.end());
    }

    /**
     * @brief Get the minimum stored value.
     */
    T minimum() const {
        if (m_data.empty()) return T{};
        return *std::min_element(m_data.begin(), m_data.end());
    }

    /**
     * @brief Filter values matching a lambda predicate (uses STL copy_if).
     * @param predicate Lambda returning true for elements to include.
     * @return Vector of matching elements.
     *
     * @example
     * auto highs = buffer.filter([](double v){ return v > 100.0; });
     */
    std::vector<T> filter(std::function<bool(const T&)> predicate) const {
        std::vector<T> result;
        std::copy_if(m_data.begin(), m_data.end(),
                     std::back_inserter(result), predicate);
        return result;
    }

    /**
     * @brief Count how many values satisfy the predicate (uses STL count_if).
     */
    size_t countIf(std::function<bool(const T&)> predicate) const {
        return static_cast<size_t>(
            std::count_if(m_data.begin(), m_data.end(), predicate));
    }

    /** @brief Get the most recently added value */
    const T& latest() const {
        if (m_data.empty()) throw std::runtime_error("SensorDataBuffer is empty");
        return m_data.back();
    }

    /** @brief Return all data as a const reference */
    const std::vector<T>& data() const { return m_data; }

    /** @brief Number of stored elements */
    size_t size() const { return m_data.size(); }

    /** @brief Maximum capacity of the buffer */
    size_t capacity() const { return m_capacity; }

    /** @brief True if no elements have been recorded yet */
    bool empty() const { return m_data.empty(); }

    /** @brief Clear all stored elements */
    void clear() { m_data.clear(); }

    /**
     * @brief Format buffer summary as a human-readable string.
     * Useful for dashboard display and logging.
     */
    std::string summary() const {
        if (m_data.empty()) return "[empty]";
        std::ostringstream oss;
        oss << "n=" << m_data.size()
            << " avg=" << average()
            << " min=" << minimum()
            << " max=" << maximum();
        return oss.str();
    }

private:
    size_t m_capacity;
    std::vector<T> m_data;
};

} // namespace VehicleSystem
