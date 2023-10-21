//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_IOUTILS_H
#define UDPFWD_IOUTILS_H

#include <vector>
#include <cstdint>
#include <string_view>
#include <string>


namespace platform::io {

bool IsFileOrDirExists(std::string_view path);

bool IsRegularFileExists(std::string_view path);

bool IsDirectoryExists(std::string_view path);

/**
 * Read file content into a vector
 * @param content output
 * @return 0 if success, -errno if failed
 */
int ReadFully(std::vector<uint8_t>& content, std::string_view path);

int ReadAtMostOrFail(std::vector<uint8_t>& content, std::string_view path, size_t max);

int ReadFullyAsString8(std::string& content, std::string_view path);

int ReadAsString8AtMostOrFail(std::string& content, std::string_view path, size_t max);


namespace sizesuffix {

constexpr uint64_t operator "" _B(unsigned long long int n) {
    return n;
}

constexpr uint64_t operator "" _KB(unsigned long long int n) {
    return n * 1024;
}

constexpr uint64_t operator "" _MB(unsigned long long int n) {
    return n * 1024 * 1024;
}

constexpr uint64_t operator "" _GB(unsigned long long int n) {
    return n * 1024 * 1024 * 1024;
}

}

}

#endif //UDPFWD_IOUTILS_H
