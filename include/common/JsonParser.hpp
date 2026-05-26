#pragma once

#include <string>
#include <map>
#include <vector>
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
    enum class Type { Null, Bool, Int, Double, String, Array, Object };

    JsonValue() : m_type(Type::Null) {}
    JsonValue(std::nullptr_t) : m_type(Type::Null) {}
    JsonValue(bool val) : m_type(Type::Bool), m_bool(val) {}
    JsonValue(int val) : m_type(Type::Int), m_int(val) {}
    JsonValue(double val) : m_type(Type::Double), m_double(val) {}
    JsonValue(const std::string& val) : m_type(Type::String), m_string(val) {}
    JsonValue(const char* val) : m_type(Type::String), m_string(val) {}
    JsonValue(const Array& val) : m_type(Type::Array) { m_array = new Array(val); }
    JsonValue(const Object& val) : m_type(Type::Object) { m_object = new Object(val); }

    // Copy semantics needed because of raw pointers
    JsonValue(const JsonValue& other) : m_type(other.m_type), m_bool(other.m_bool),
        m_int(other.m_int), m_double(other.m_double), m_string(other.m_string) {
        if (other.m_array) m_array = new Array(*other.m_array);
        else m_array = nullptr;
        if (other.m_object) m_object = new Object(*other.m_object);
        else m_object = nullptr;
    }

    JsonValue& operator=(const JsonValue& other) {
        if (this != &other) {
            delete m_array; m_array = nullptr;
            delete m_object; m_object = nullptr;
            m_type = other.m_type;
            m_bool = other.m_bool;
            m_int = other.m_int;
            m_double = other.m_double;
            m_string = other.m_string;
            if (other.m_array) m_array = new Array(*other.m_array);
            if (other.m_object) m_object = new Object(*other.m_object);
        }
        return *this;
    }

    ~JsonValue() {
        delete m_array;
        delete m_object;
    }

    // --- Type checks ---
    bool isNull() const { return m_type == Type::Null; }
    bool isBool() const { return m_type == Type::Bool; }
    bool isInt() const { return m_type == Type::Int; }
    bool isDouble() const { return m_type == Type::Double; }
    bool isNumber() const { return isInt() || isDouble(); }
    bool isString() const { return m_type == Type::String; }
    bool isArray() const { return m_type == Type::Array; }
    bool isObject() const { return m_type == Type::Object; }

    // --- Getters ---
    bool getBool() const { return m_bool; }
    int getInt() const {
        if (isInt()) return m_int;
        if (isDouble()) return static_cast<int>(m_double);
        throw std::runtime_error("JSON value is not a number");
    }
    double getDouble() const {
        if (isDouble()) return m_double;
        if (isInt()) return static_cast<double>(m_int);
        throw std::runtime_error("JSON value is not a number");
    }
    const std::string& getString() const { return m_string; }
    const Array& getArray() const { return *m_array; }
    const Object& getObject() const { return *m_object; }

    // --- Getters with default ---
    bool get(bool defaultVal) const {
        try { return getBool(); } catch (...) { return defaultVal; }
    }
    int get(int defaultVal) const {
        try { return getInt(); } catch (...) { return defaultVal; }
    }
    double get(double defaultVal) const {
        try { return getDouble(); } catch (...) { return defaultVal; }
    }
    std::string get(const std::string& defaultVal) const {
        try { return getString(); } catch (...) { return defaultVal; }
    }
    std::string get(const char* defaultVal) const {
        try { return getString(); } catch (...) { return std::string(defaultVal); }
    }

    // --- Object access ---
    const JsonValue& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Not a JSON object");
        auto it = m_object->find(key);
        if (it == m_object->end()) {
            static JsonValue nullVal;
            return nullVal;
        }
        return it->second;
    }

    bool hasKey(const std::string& key) const {
        if (!isObject()) return false;
        return m_object->count(key) > 0;
    }

    // --- Array access ---
    const JsonValue& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Not a JSON array");
        return m_array->at(index);
    }

    size_t size() const {
        if (isArray()) return m_array->size();
        if (isObject()) return m_object->size();
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
    Type m_type;
    bool m_bool = false;
    int m_int = 0;
    double m_double = 0.0;
    std::string m_string;
    Array* m_array = nullptr;
    Object* m_object = nullptr;

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
