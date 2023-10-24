//
// Created by sulfate on 10/20/23.
//
#include "ParseNumber.h"

#include <limits>
#include <type_traits>

#include "cstdlib"

namespace support::text {

bool ParseInt64(std::string_view str, int64_t& value) {
    int64_t result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtoll(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}

bool ParseInt(std::string_view str, int& value) {
    // check int and int32 are the same
    static_assert(std::is_same_v<int, int32_t>, "int and int32_t are not the same");
    return ParseInt32(str, value);
}

bool ParseInt32(std::string_view str, int32_t& value) {
    char* end;
    std::string str2 = std::string(str);
    auto result = std::strtol(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        // check signed long to int32_t overflow
        if (result < std::numeric_limits<int32_t>::min() || result > std::numeric_limits<int32_t>::max()) {
            return false;
        } else {
            value = (int32_t) result;
            return true;
        }
    } else {
        return false;
    }
}


bool ParseUInt32(std::string_view str, uint32_t& value) {
    char* end;
    std::string str2 = std::string(str);
    auto result = std::strtoul(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        // check unsigned long to uint32_t overflow
        if (result > std::numeric_limits<uint32_t>::max()) {
            return false;
        } else {
            value = (uint32_t) result;
            return true;
        }
    } else {
        return false;
    }
}

bool ParseUInt64(std::string_view str, uint64_t& value) {
    uint64_t result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtoull(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}

bool ParseDouble(std::string_view str, double& value) {
    double result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtod(str.data(), &end);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}

bool ParseFloat(std::string_view str, float& value) {
    float result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtof(str.data(), &end);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}

bool ParseUInt16(std::string_view str, uint16_t& value) {
    int32_t result;
    if (!ParseInt32(str, result)) {
        return false;
    }
    // check range
    if (result < 0 || result > std::numeric_limits<uint16_t>::max()) {
        return false;
    }
    value = (uint16_t) result;
    return true;
}

bool ParseInt16(std::string_view str, int16_t& value) {
    int32_t result;
    if (!ParseInt32(str, result)) {
        return false;
    }
    // check range
    if (result < std::numeric_limits<int16_t>::min() || result > std::numeric_limits<int16_t>::max()) {
        return false;
    }
    value = (int16_t) result;
    return true;
}

std::optional<int> ParseInt(std::string_view str) {
    int result = 0;
    if (!ParseInt(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<int32_t> ParseInt32(std::string_view str) {
    int32_t result = 0;
    if (!ParseInt32(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<int64_t> ParseInt64(std::string_view str) {
    int64_t result = 0;
    if (!ParseInt64(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<uint32_t> ParseUInt32(std::string_view str) {
    uint32_t result = 0;
    if (!ParseUInt32(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<uint64_t> ParseUInt64(std::string_view str) {
    uint64_t result = 0;
    if (!ParseUInt64(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<uint16_t> ParseUInt16(std::string_view str) {
    uint16_t result = 0;
    if (!ParseUInt16(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<int16_t> ParseInt16(std::string_view str) {
    int16_t result = 0;
    if (!ParseInt16(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<double> ParseDouble(std::string_view str) {
    double result = 0;
    if (!ParseDouble(str, result)) {
        return std::nullopt;
    }
    return result;
}

std::optional<float> ParseFloat(std::string_view str) {
    float result = 0;
    if (!ParseFloat(str, result)) {
        return std::nullopt;
    }
    return result;
}

}
