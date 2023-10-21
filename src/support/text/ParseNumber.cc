//
// Created by sulfate on 10/20/23.
//
#include "ParseNumber.h"

#include <limits>

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
    int result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtol(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}

bool ParseInt32(std::string_view str, int32_t& value) {
    int32_t result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtol(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
    } else {
        return false;
    }
}


bool ParseUInt32(std::string_view str, uint32_t& value) {
    uint32_t result;
    char* end;
    std::string str2 = std::string(str);
    result = std::strtoul(str.data(), &end, 10);
    if (end == str.data() + str.size()) {
        value = result;
        return true;
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

}
