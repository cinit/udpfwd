//
// Created by sulfate on 10/20/23.
//

#include "IoUtils.h"

#include <cstdint>
#include <fcntl.h>
#include <array>

#include <unistd.h>
#include <sys/stat.h>

namespace platform::io {

/**
 * Read file content into a vector
 * @param content output
 * @return 0 if success, -errno if failed
 */
int ReadFully(std::vector<uint8_t>& content, std::string_view path) {
    // get file size
    struct stat st{};
    auto cpath = std::string(path);
    if (stat(cpath.c_str(), &st) != 0) {
        return -errno;
    }
    content.clear();
    content.reserve(st.st_size);
    // open file
    int fd = open(cpath.c_str(), O_RDONLY);
    if (fd < 0) {
        return -errno;
    }
    // read
    ssize_t readBytes;
    std::array<uint8_t, 4096> buf{};
    while (true) {
        readBytes = read(fd, buf.data(), buf.size());
        if (readBytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -errno;
        } else if (readBytes == 0) {
            break;
        } else {
            content.insert(content.end(), buf.begin(), buf.begin() + readBytes);
        }
    }
    close(fd);
    return 0;
}

int ReadAtMostOrFail(std::vector<uint8_t>& content, std::string_view path, size_t max) {
    // stat first
    auto cpath = std::string(path);
    struct stat st{};
    if (stat(cpath.c_str(), &st) != 0) {
        return -errno;
    }
    content.clear();
    if (st.st_size > 1) {
        // use min
        size_t size = std::min((size_t) st.st_size, max);
        content.reserve(size);
    }
    // open file
    int fd = open(cpath.c_str(), O_RDONLY);
    if (fd < 0) {
        return -errno;
    }
    // read
    ssize_t readBytes;
    ssize_t remaining = (ssize_t) max;
    std::array<uint8_t, 4096> buf{};
    while (remaining > 0) {
        readBytes = read(fd, buf.data(), std::min((size_t) remaining, buf.size()));
        if (readBytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -errno;
        } else if (readBytes == 0) {
            break;
        } else {
            content.insert(content.end(), buf.begin(), buf.begin() + readBytes);
            remaining -= readBytes;
        }
    }
    close(fd);
    return 0;
}

int ReadFullyAsString8(std::string& content, std::string_view path) {
    std::vector<uint8_t> buf;
    int ret = ReadFully(buf, path);
    if (ret != 0) {
        return ret;
    }
    content = std::string(buf.begin(), buf.end());
    return 0;
}

int ReadAsString8AtMostOrFail(std::string& content, std::string_view path, size_t max) {
    std::vector<uint8_t> buf;
    int ret = ReadAtMostOrFail(buf, path, max);
    if (ret != 0) {
        return ret;
    }
    content = std::string(buf.begin(), buf.end());
    return 0;
}


bool IsFileOrDirExists(std::string_view path) {
    struct stat st{};
    auto cpath = std::string(path);
    return stat(cpath.c_str(), &st) == 0;
}

bool IsRegularFileExists(std::string_view path) {
    struct stat st{};
    auto cpath = std::string(path);
    if (stat(cpath.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

bool IsDirectoryExists(std::string_view path) {
    struct stat st{};
    auto cpath = std::string(path);
    if (stat(cpath.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);

}

}
