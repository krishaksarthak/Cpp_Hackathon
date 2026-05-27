#pragma once

#include <string>

namespace VehicleSystem {
namespace Colors {

const std::string RESET   = "\033[0m";
const std::string BOLD    = "\033[1m";

const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string WHITE   = "\033[37m";

const std::string BOLD_RED     = "\033[1;31m";
const std::string BOLD_GREEN   = "\033[1;32m";
const std::string BOLD_YELLOW  = "\033[1;33m";
const std::string BOLD_BLUE    = "\033[1;34m";
const std::string BOLD_MAGENTA = "\033[1;35m";
const std::string BOLD_CYAN    = "\033[1;36m";
const std::string BOLD_WHITE   = "\033[1;37m";

const std::string BG_RED       = "\033[41m";
const std::string BG_GREEN     = "\033[42m";
const std::string BG_YELLOW    = "\033[43m";
const std::string BG_BLUE      = "\033[44m";
const std::string BG_MAGENTA   = "\033[45m";
const std::string BG_CYAN      = "\033[46m";
const std::string BG_WHITE     = "\033[47m";

} // namespace Colors
} // namespace VehicleSystem
