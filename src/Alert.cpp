#include "Alert.hpp"

Alert::Alert(std::string msg, Severity sev)
    : message(msg), severity(sev) {}

std::string Alert::getMessage() const {
    return message;
}

Severity Alert::getSeverity() const {
    return severity;
}

std::ostream& operator<<(std::ostream& os, const Alert& alert) {
    os << "[ALERT] " << alert.message;
    return os;
}
