#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace VehicleSystem {

/**
 * @brief Lightweight JSON parser with no external dependencies.
 * 
 * Demonstrates: Templates, variant types, exception handling,
 * recursive descent parsing, file I/O.
 */
class JsonValue {
public:
    using Object = std::map<std::string, JsonValue>;
    using Array = std::vector<JsonValue>;
    using Value = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Object>;

    JsonValue() : m_value(nullptr) {}
    JsonValue(std::nullptr_t) : m_value(nullptr) {}
    JsonValue(bool val) : m_value(val) {}
    JsonValue(int val) : m_value(val) {}
    JsonValue(double val) : m_value(val) {}
    JsonValue(const std::string& val) : m_value(val) {}
    JsonValue(const char* val) : m_value(std::string(val)) {}
    JsonValue(const Array& val) : m_value(val) {}
    JsonValue(const Object& val) : m_value(val) {}

    // --- Type checks ---
    bool isNull() const { return std::holds_alternative<std::nullptr_t>(m_value); }
    bool isBool() const { return std::holds_alternative<bool>(m_value); }
    bool isInt() const { return std::holds_alternative<int>(m_value); }
    bool isDouble() const { return std::holds_alternative<double>(m_value); }
    bool isNumber() const { return isInt() || isDouble(); }
    bool isString() const { return std::holds_alternative<std::string>(m_value); }
    bool isArray() const { return std::holds_alternative<Array>(m_value); }
    bool isObject() const { return std::holds_alternative<Object>(m_value); }

    // --- Getters ---
    bool getBool() const { return std::get<bool>(m_value); }
    int getInt() const {
        if (isInt()) return std::get<int>(m_value);
        if (isDouble()) return static_cast<int>(std::get<double>(m_value));
        throw std::runtime_error("JSON value is not a number");
    }
    double getDouble() const {
        if (isDouble()) return std::get<double>(m_value);
        if (isInt()) return static_cast<double>(std::get<int>(m_value));
        throw std::runtime_error("JSON value is not a number");
    }
    const std::string& getString() const { return std::get<std::string>(m_value); }
    const Array& getArray() const { return std::get<Array>(m_value); }
    const Object& getObject() const { return std::get<Object>(m_value); }

    // --- Template getter with default ---
    template<typename T>
    T get(const T& defaultVal = T{}) const {
        try {
            if constexpr (std::is_same_v<T, bool>) return getBool();
            else if constexpr (std::is_same_v<T, int>) return getInt();
            else if constexpr (std::is_same_v<T, double>) return getDouble();
            else if constexpr (std::is_same_v<T, std::string>) return getString();
            else return defaultVal;
        } catch (...) {
            return defaultVal;
        }
    }

    // --- Object access ---
    const JsonValue& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Not a JSON object");
        auto it = std::get<Object>(m_value).find(key);
        if (it == std::get<Object>(m_value).end()) {
            static JsonValue nullVal;
            return nullVal;
        }
        return it->second;
    }

    bool hasKey(const std::string& key) const {
        if (!isObject()) return false;
        return std::get<Object>(m_value).count(key) > 0;
    }

    // --- Array access ---
    const JsonValue& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Not a JSON array");
        return std::get<Array>(m_value).at(index);
    }

    size_t size() const {
        if (isArray()) return std::get<Array>(m_value).size();
        if (isObject()) return std::get<Object>(m_value).size();
        return 0;
    }

    // --- Static parse methods ---
    static JsonValue parse(const std::string& jsonStr) {
        size_t pos = 0;
        return parseValue(jsonStr, pos);
    }

    static JsonValue parseFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open JSON file: " + filePath);
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        return parse(content);
    }

private:
    Value m_value;

    static void skipWhitespace(const std::string& s, size_t& pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' ||
               s[pos] == '\n' || s[pos] == '\r')) {
            ++pos;
        }
    }

    static JsonValue parseValue(const std::string& s, size_t& pos) {
        skipWhitespace(s, pos);
        if (pos >= s.size()) throw std::runtime_error("Unexpected end of JSON");

        char c = s[pos];
        if (c == '"') return parseString(s, pos);
        if (c == '{') return parseObject(s, pos);
        if (c == '[') return parseArray(s, pos);
        if (c == 't' || c == 'f') return parseBool(s, pos);
        if (c == 'n') return parseNull(s, pos);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(s, pos);

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    static JsonValue parseString(const std::string& s, size_t& pos) {
        ++pos; // skip opening quote
        std::string result;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\') {
                ++pos;
                if (pos >= s.size()) throw std::runtime_error("Unexpected end in string escape");
                switch (s[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += s[pos]; break;
                }
            } else {
                result += s[pos];
            }
            ++pos;
        }
        if (pos >= s.size()) throw std::runtime_error("Unterminated string");
        ++pos; // skip closing quote
        return JsonValue(result);
    }

    static JsonValue parseNumber(const std::string& s, size_t& pos) {
        size_t start = pos;
        bool isFloat = false;
        if (s[pos] == '-') ++pos;
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        if (pos < s.size() && s[pos] == '.') {
            isFloat = true;
            ++pos;
            while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            isFloat = true;
            ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
            while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        }
        std::string numStr = s.substr(start, pos - start);
        if (isFloat) {
            return JsonValue(std::stod(numStr));
        } else {
            try {
                return JsonValue(std::stoi(numStr));
            } catch (...) {
                return JsonValue(std::stod(numStr));
            }
        }
    }

    static JsonValue parseObject(const std::string& s, size_t& pos) {
        ++pos; // skip '{'
        Object obj;
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') { ++pos; return JsonValue(obj); }

        while (true) {
            skipWhitespace(s, pos);
            if (s[pos] != '"') throw std::runtime_error("Expected string key in object");
            auto key = parseString(s, pos);
            skipWhitespace(s, pos);
            if (s[pos] != ':') throw std::runtime_error("Expected ':' in object");
            ++pos;
            obj[key.getString()] = parseValue(s, pos);
            skipWhitespace(s, pos);
            if (s[pos] == '}') { ++pos; break; }
            if (s[pos] != ',') throw std::runtime_error("Expected ',' or '}' in object");
            ++pos;
        }
        return JsonValue(obj);
    }

    static JsonValue parseArray(const std::string& s, size_t& pos) {
        ++pos; // skip '['
        Array arr;
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') { ++pos; return JsonValue(arr); }

        while (true) {
            arr.push_back(parseValue(s, pos));
            skipWhitespace(s, pos);
            if (s[pos] == ']') { ++pos; break; }
            if (s[pos] != ',') throw std::runtime_error("Expected ',' or ']' in array");
            ++pos;
        }
        return JsonValue(arr);
    }

    static JsonValue parseBool(const std::string& s, size_t& pos) {
        if (s.substr(pos, 4) == "true") { pos += 4; return JsonValue(true); }
        if (s.substr(pos, 5) == "false") { pos += 5; return JsonValue(false); }
        throw std::runtime_error("Invalid boolean value");
    }

    static JsonValue parseNull(const std::string& s, size_t& pos) {
        if (s.substr(pos, 4) == "null") { pos += 4; return JsonValue(nullptr); }
        throw std::runtime_error("Invalid null value");
    }
};

} // namespace VehicleSystem
