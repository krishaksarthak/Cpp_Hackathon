#pragma once
#include <string>
#include <iostream>

enum class Severity {
    INFO,
    WARNING,
    CRITICAL
};

class Alert {
    std::string message;
    Severity severity;

public:
    Alert(std::string msg, Severity sev);

    std::string getMessage() const;
    Severity getSeverity() const;

    friend std::ostream& operator<<(std::ostream& os, const Alert& alert);
};
