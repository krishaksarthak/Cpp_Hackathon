#include "logging/EventLogger.hpp"

namespace VehicleSystem {

std::string LogEntry::format() const {
    std::ostringstream oss;
    oss << "[" << Utils::timestampToString(timestamp) << "] "
        << "[" << std::setw(8) << severityToString(severity) << "] ";
    if (!source.empty()) {
        oss << "[" << source << "] ";
    }
    oss << message;
    return oss.str();
}

EventLogger::EventLogger(const std::string& logFilePath)
    : m_logFilePath(logFilePath) {
    m_logFile.open(logFilePath, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + logFilePath);
    }

    // Derive directory and open additional logs
    std::string logDir = "logs";
    auto pos = logFilePath.find_last_of("/\\");
    if (pos != std::string::npos) {
        logDir = logFilePath.substr(0, pos);
    }

    m_dtcLogFile.open(logDir + "/dtc_history.log", std::ios::out | std::ios::app);
    m_perfLogFile.open(logDir + "/performance_metrics.log", std::ios::out | std::ios::app);

    logEvent(AlertSeverity::INFO, "Event Logger initialized", "SYSTEM");
}

EventLogger::~EventLogger() {
    try {
        logEvent(AlertSeverity::INFO, "Event Logger shutting down", "SYSTEM");
        flush();
        if (m_logFile.is_open()) m_logFile.close();
        if (m_dtcLogFile.is_open()) m_dtcLogFile.close();
        if (m_perfLogFile.is_open()) m_perfLogFile.close();
    } catch (...) {
        // Suppress exceptions in destructor
    }
}

void EventLogger::logEvent(AlertSeverity severity, const std::string& message,
              const std::string& source) {
    LogEntry entry{std::chrono::system_clock::now(), severity, message, source};

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_logQueue.push(entry);
}

void EventLogger::logAlert(const Alert& alert) {
    logEvent(alert.getSeverity(), alert.getMessage(),
             sensorTypeToString(alert.getSource()));
}

void EventLogger::logDTC(const std::string& dtcData) {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_dtcLogFile.is_open()) {
        m_dtcLogFile << "[" << Utils::timestampToString(std::chrono::system_clock::now()) << "] "
                     << dtcData << "\n";
        m_dtcLogFile.flush();
    }
}

void EventLogger::logPerformance(const std::string& perfData) {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_perfLogFile.is_open()) {
        m_perfLogFile << "=== Performance Snapshot [" 
                      << Utils::timestampToString(std::chrono::system_clock::now()) << "] ===\n"
                      << perfData << "\n";
        m_perfLogFile.flush();
    }
}

void EventLogger::processQueue() {
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

std::vector<LogEntry> EventLogger::searchEvents(
    std::function<bool(const LogEntry&)> predicate) const {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    std::vector<LogEntry> results;
    // STL algorithm: std::copy_if with lambda predicate
    std::copy_if(m_logHistory.begin(), m_logHistory.end(),
                 std::back_inserter(results), predicate);
    return results;
}

std::vector<LogEntry> EventLogger::getRecentEvents(size_t count) const {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    if (m_logHistory.size() <= count) {
        return m_logHistory;
    }
    return std::vector<LogEntry>(m_logHistory.end() - count, m_logHistory.end());
}

size_t EventLogger::getTotalLogCount() const {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    return m_logHistory.size();
}

size_t EventLogger::getPendingCount() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_logQueue.size();
}

void EventLogger::flush() {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_logFile.is_open()) m_logFile.flush();
    if (m_dtcLogFile.is_open()) m_dtcLogFile.flush();
    if (m_perfLogFile.is_open()) m_perfLogFile.flush();
}

} // namespace VehicleSystem
