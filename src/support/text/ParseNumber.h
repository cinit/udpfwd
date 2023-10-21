//
// Created by sulfate on 10/20/23.
//

#include <string>
#include <string_view>
#include <cstdint>

namespace support::text {

bool ParseInt(std::string_view str, int& value);

bool ParseInt32(std::string_view str, int32_t& value);

bool ParseInt64(std::string_view str, int64_t& value);

bool ParseUInt32(std::string_view str, uint32_t& value);

bool ParseUInt64(std::string_view str, uint64_t& value);

bool ParseUInt16(std::string_view str, uint16_t& value);

bool ParseInt16(std::string_view str, int16_t& value);

bool ParseDouble(std::string_view str, double& value);

bool ParseFloat(std::string_view str, float& value);

}
