//
// Created by sulfate on 8/31/23.
//

#include "Log.h"

#include <unistd.h>

#include "platform/Time.h"

namespace platform::log {

static void DefaultLogToStderrHandler(Log::Level level, std::string_view tag, std::string_view msg) noexcept;

constinit volatile Log::LogHandler Log::sLogHandler = &DefaultLogToStderrHandler;

static void DefaultLogToStderrHandler(Log::Level level, std::string_view tag, std::string_view msg) noexcept {
    auto time = time::GetCurrentLocalDateTime();
    char levelChar;
    switch (level) {
        using Level = log::Log::Level;
        case Level::VERBOSE:
            levelChar = 'V';
            break;
        case Level::DEBUG:
            levelChar = 'D';
            break;
        case Level::INFO:
            levelChar = 'I';
            break;
        case Level::WARN:
            levelChar = 'W';
            break;
        case Level::ERROR:
            levelChar = 'E';
            break;
        default:
            levelChar = '?';
            break;
    }
    // format
    // MM-dd HH:mm:ss.SSSSSS L TAG MSG
    auto line = fmt::format("{:02}-{:02} {:02}:{:02}:{:02}.{:03}{:03} {} {}: {}\n",
                            time.month, time.day, time.hour, time.minute, time.second, time.millisecond, time.microsecond,
                            levelChar, tag, msg);
    write(STDERR_FILENO, line.data(), line.size());
    // ignore error
}

} // namespace log
