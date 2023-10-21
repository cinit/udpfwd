//
// Created by sulfate on 10/20/23.
//

#include "TextUtils.h"

#include <iterator>

namespace support::text {

std::vector<std::string> SplitString(std::string_view str, std::string_view delimiter) {
    std::vector<std::string> result;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        result.emplace_back(str.substr(0, pos));
        str.remove_prefix(pos + delimiter.length());
    }
    result.emplace_back(str);
    return result;
}

inline bool IsPrintableAscii(char c) noexcept {
    return c >= 0x20 && c <= 0x7E;
}

std::string LastPartOf(std::string_view str, std::string_view delimiter) {
    auto parts = SplitString(str, delimiter);
    return parts[parts.size() - 1];
}

std::vector<std::string> SplitStringWithRegex(std::string_view str, const std::regex& delimiter) {
    std::vector<std::string> result;
    std::string content = std::string(str);
    std::sregex_token_iterator iter(content.begin(), content.end(), delimiter, -1);
    std::sregex_token_iterator end;
    for (; iter != end; ++iter) {
        result.emplace_back(*iter);
    }
    return result;
}

}
