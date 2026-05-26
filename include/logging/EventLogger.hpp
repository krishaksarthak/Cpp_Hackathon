#pragma once

#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <mutex>
#include <functional>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "common/Types.hpp"
#include "common/Utils.hpp"
#include "alerts/Alert.hpp"

namespace VehicleSystem {

/**
 * @brief Represents a single log entry with timestamp and severity.
 */
struct LogEntry {
    Timestamp timestamp;
    AlertSeverity severity;
    std::string message;
    std::string source;

    /** @brief Format log entry for file output */
    std::string format() const;
};

/**
 * @brief Thread-safe event logger with file output and search capabilities.
 * 
 * Demonstrates: RAII (file handle management), thread-safe queue,
 * lambda-based filtering (STL algorithms), exception handling,
 * crash-safe logging (flush after each write).
 * Maps to Diagnostic Event Manager (DEM) in AUTOSAR.
 */
class EventLogger {
public:
    /**
     * @brief Construct logger (RAII - opens file)
     * @param logFilePath Path to the log file
     * @throws std::runtime_error if file cannot be opened
     */
    explicit EventLogger(const std::string& logFilePath);

    /**
     * @brief RAII destructor - flushes and closes file
     */
    ~EventLogger();

    // Delete copy (RAII - unique file ownership)
    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    /**
     * @brief Log an event (thread-safe, adds to queue)
     */
    void logEvent(AlertSeverity severity, const std::string& message,
                  const std::string& source = "");

    /**
     * @brief Log an alert (convenience method)
     */
    void logAlert(const Alert& alert);

    /**
     * @brief Log Diagnostic Trouble Codes to dtc_history.log
     */
    void logDTC(const std::string& dtcData);

    /**
     * @brief Log system performance metrics to performance_metrics.log
     */
    void logPerformance(const std::string& perfData);

    /**
     * @brief Process all queued log entries (called by logger thread)
     * Writes entries to file and stores in history.
     */
    void processQueue();

    /**
     * @brief Search log history using lambda predicate.
     * 
     * Demonstrates: STL algorithm (copy_if) with lambda expression.
     * 
     * @param predicate Lambda function that returns true for matching entries
     * @return Vector of matching log entries
     */
    std::vector<LogEntry> searchEvents(
        std::function<bool(const LogEntry&)> predicate) const;

    /**
     * @brief Get recent log entries
     * @param count Number of recent entries to return
     */
    std::vector<LogEntry> getRecentEvents(size_t count) const;

    /**
     * @brief Get total number of logged events
     */
    size_t getTotalLogCount() const;

    /**
     * @brief Get count of pending (unprocessed) log entries
     */
    size_t getPendingCount() const;

    /**
     * @brief Flush the log file
     */
    void flush();

    /**
     * @brief Get the log file path
     */
    std::string getLogFilePath() const { return m_logFilePath; }

private:
    std::string m_logFilePath;
    std::ofstream m_logFile;
    
    std::ofstream m_dtcLogFile;
    std::ofstream m_perfLogFile;

    std::queue<LogEntry> m_logQueue;
    std::vector<LogEntry> m_logHistory;

    mutable std::mutex m_queueMutex;
    mutable std::mutex m_fileMutex;
    mutable std::mutex m_historyMutex;
};

} // namespace VehicleSystem
