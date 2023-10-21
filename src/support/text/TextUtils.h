//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_TEXTUTILS_H
#define UDPFWD_TEXTUTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <regex>

namespace support::text {

std::vector<std::string> SplitString(std::string_view str, std::string_view delimiter);

std::vector<std::string> SplitStringWithRegex(std::string_view str, const std::regex& delimiter);

bool IsPrintableAscii(char c) noexcept;

std::string LastPartOf(std::string_view str, std::string_view delimiter);

} // namespace support::text

#endif //UDPFWD_TEXTUTILS_H
