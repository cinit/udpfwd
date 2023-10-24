//
// Created by sulfate on 10/20/23.
//

#include <string>
#include <string_view>
#include <cstdint>
#include <optional>

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

std::optional<int> ParseInt(std::string_view str);

std::optional<int32_t> ParseInt32(std::string_view str);

std::optional<int64_t> ParseInt64(std::string_view str);

std::optional<uint32_t> ParseUInt32(std::string_view str);

std::optional<uint64_t> ParseUInt64(std::string_view str);

std::optional<uint16_t> ParseUInt16(std::string_view str);

std::optional<int16_t> ParseInt16(std::string_view str);

std::optional<double> ParseDouble(std::string_view str);

std::optional<float> ParseFloat(std::string_view str);

}
