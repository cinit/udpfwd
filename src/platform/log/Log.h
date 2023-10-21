//
// Created by sulfate on 8/31/23.
//

#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <string_view>
#include <fmt/format.h>

namespace platform::log {

class Log {
public:
    enum class Level {
        UNKNOWN = 0,
        VERBOSE = 2,
        DEBUG = 3,
        INFO = 4,
        WARN = 5,
        ERROR = 6
    };
    using LogHandler = void (*)(Level level, std::string_view tag, std::string_view msg) noexcept;

private:
    static volatile LogHandler sLogHandler;
public:

    static void LogMessage(Level level, std::string_view tag, std::string_view msg) noexcept {
        LogHandler h = sLogHandler;
        if (h == nullptr) {
            return;
        }
        h(level, tag, msg);
    }

    static inline LogHandler GetLogHandler() noexcept {
        return sLogHandler;
    }

    static inline void SetLogHandler(LogHandler h) noexcept {
        sLogHandler = h;
    }
};

} // namespace log

// macros

#define LOGV(...) ::platform::log::Log::LogMessage(::platform::log::Log::Level::VERBOSE, LOG_TAG, ::fmt::format(__VA_ARGS__))
#define LOGD(...) ::platform::log::Log::LogMessage(::platform::log::Log::Level::DEBUG, LOG_TAG, ::fmt::format(__VA_ARGS__))
#define LOGI(...) ::platform::log::Log::LogMessage(::platform::log::Log::Level::INFO, LOG_TAG, ::fmt::format(__VA_ARGS__))
#define LOGW(...) ::platform::log::Log::LogMessage(::platform::log::Log::Level::WARN, LOG_TAG, ::fmt::format(__VA_ARGS__))
#define LOGE(...) ::platform::log::Log::LogMessage(::platform::log::Log::Level::ERROR, LOG_TAG, ::fmt::format(__VA_ARGS__))

#endif //UTIL_LOG_H
