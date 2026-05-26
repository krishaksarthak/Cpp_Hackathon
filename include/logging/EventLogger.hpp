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
    std::string format() const {
        std::ostringstream oss;
        oss << "[" << Utils::timestampToString(timestamp) << "] "
            << "[" << std::setw(8) << severityToString(severity) << "] ";
        if (!source.empty()) {
            oss << "[" << source << "] ";
        }
        oss << message;
        return oss.str();
    }
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
    explicit EventLogger(const std::string& logFilePath)
        : m_logFilePath(logFilePath) {
        m_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + logFilePath);
        }
        logEvent(AlertSeverity::INFO, "Event Logger initialized", "SYSTEM");
    }

    /**
     * @brief RAII destructor - flushes and closes file
     */
    ~EventLogger() {
        try {
            logEvent(AlertSeverity::INFO, "Event Logger shutting down", "SYSTEM");
            flush();
            if (m_logFile.is_open()) {
                m_logFile.close();
            }
        } catch (...) {
            // Suppress exceptions in destructor
        }
    }

    // Delete copy (RAII - unique file ownership)
    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    /**
     * @brief Log an event (thread-safe, adds to queue)
     */
    void logEvent(AlertSeverity severity, const std::string& message,
                  const std::string& source = "") {
        LogEntry entry{std::chrono::system_clock::now(), severity, message, source};

        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_logQueue.push(entry);
    }

    /**
     * @brief Log an alert (convenience method)
     */
    void logAlert(const Alert& alert) {
        logEvent(alert.getSeverity(), alert.getMessage(),
                 sensorTypeToString(alert.getSource()));
    }

    /**
     * @brief Process all queued log entries (called by logger thread)
     * Writes entries to file and stores in history.
     */
    void processQueue() {
        std::queue<LogEntry> toProcess;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            std::swap(toProcess, m_logQueue);
        }

        std::lock_guard<std::mutex> lock(m_fileMutex);
        while (!toProcess.empty()) {
            const auto& entry = toProcess.front();
            
            // Write to file (crash-safe: flush immediately)
            if (m_logFile.is_open()) {
                m_logFile << entry.format() << "\n";
                m_logFile.flush();
            }

            // Store in history
            {
                std::lock_guard<std::mutex> histLock(m_historyMutex);
                m_logHistory.push_back(entry);
            }

            toProcess.pop();
        }
    }

    /**
     * @brief Search log history using lambda predicate.
     * 
     * Demonstrates: STL algorithm (copy_if) with lambda expression.
     * 
     * @param predicate Lambda function that returns true for matching entries
     * @return Vector of matching log entries
     */
    std::vector<LogEntry> searchEvents(
        std::function<bool(const LogEntry&)> predicate) const {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        std::vector<LogEntry> results;
        // STL algorithm: std::copy_if with lambda predicate
        std::copy_if(m_logHistory.begin(), m_logHistory.end(),
                     std::back_inserter(results), predicate);
        return results;
    }

    /**
     * @brief Get recent log entries
     * @param count Number of recent entries to return
     */
    std::vector<LogEntry> getRecentEvents(size_t count) const {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        if (m_logHistory.size() <= count) {
            return m_logHistory;
        }
        return std::vector<LogEntry>(m_logHistory.end() - count, m_logHistory.end());
    }

    /**
     * @brief Get total number of logged events
     */
    size_t getTotalLogCount() const {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        return m_logHistory.size();
    }

    /**
     * @brief Get count of pending (unprocessed) log entries
     */
    size_t getPendingCount() const {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        return m_logQueue.size();
    }

    /**
     * @brief Flush the log file
     */
    void flush() {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        if (m_logFile.is_open()) {
            m_logFile.flush();
        }
    }

    /**
     * @brief Get the log file path
     */
    std::string getLogFilePath() const { return m_logFilePath; }

private:
    std::string m_logFilePath;
    std::ofstream m_logFile;
    std::queue<LogEntry> m_logQueue;
    std::vector<LogEntry> m_logHistory;

    mutable std::mutex m_queueMutex;
    mutable std::mutex m_fileMutex;
    mutable std::mutex m_historyMutex;
};

} // namespace VehicleSystem
