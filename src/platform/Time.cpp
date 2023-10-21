//
// Created by sulfate on 8/31/23.
//

#include "Time.h"

#include <ctime>
#include <sys/time.h>

namespace platform::time {

uint64_t CurrentTimeNanos() noexcept {
    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<uint64_t>(ts.tv_sec * 1000000000LLU + ts.tv_nsec);
}

uint64_t CurrentTimeMillis() noexcept {
    return CurrentTimeNanos() / 1000000LLU;
}

uint64_t CurrentTimeSeconds() noexcept {
    return CurrentTimeNanos() / 1000000000LLU;
}

int64_t GetUtcOffsetSeconds() noexcept {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    struct tm tm{};
    localtime_r(&tv.tv_sec, &tm);
    return tm.tm_gmtoff;
}

DateTime TimeSecondsToDateTime(uint64_t unixSeconds, int64_t utcOffsetSeconds) noexcept {
    struct tm tm = {};
    auto t = static_cast<time_t>(unixSeconds + utcOffsetSeconds);
    gmtime_r(&t, &tm);
    return DateTime{
            .year = static_cast<uint16_t>(tm.tm_year + 1900),
            .month = static_cast<uint8_t>(tm.tm_mon + 1),
            .day = static_cast<uint8_t>(tm.tm_mday),
            .hour = static_cast<uint8_t>(tm.tm_hour),
            .minute = static_cast<uint8_t>(tm.tm_min),
            .second = static_cast<uint8_t>(tm.tm_sec),
    };
}

DateTime TimeMillisToDateTime(uint64_t unixMillis, int64_t utcOffsetSeconds) noexcept {
    struct tm tm = {};
    auto t = static_cast<time_t>(unixMillis / 1000LLU + utcOffsetSeconds);
    gmtime_r(&t, &tm);
    return DateTime{
            .year = static_cast<uint16_t>(tm.tm_year + 1900),
            .month = static_cast<uint8_t>(tm.tm_mon + 1),
            .day = static_cast<uint8_t>(tm.tm_mday),
            .hour = static_cast<uint8_t>(tm.tm_hour),
            .minute = static_cast<uint8_t>(tm.tm_min),
            .second = static_cast<uint8_t>(tm.tm_sec),
            .millisecond = static_cast<uint16_t>(unixMillis % 1000LLU),
    };
}

DateTime TimeNanosToDateTime(uint64_t unixNanos, int64_t utcOffsetSeconds) noexcept {
    struct tm tm = {};
    auto t = static_cast<time_t>(unixNanos / 1000000000LLU + utcOffsetSeconds);
    gmtime_r(&t, &tm);
    return DateTime{
            .year = static_cast<uint16_t>(tm.tm_year + 1900),
            .month = static_cast<uint8_t>(tm.tm_mon + 1),
            .day = static_cast<uint8_t>(tm.tm_mday),
            .hour = static_cast<uint8_t>(tm.tm_hour),
            .minute = static_cast<uint8_t>(tm.tm_min),
            .second = static_cast<uint8_t>(tm.tm_sec),
            .millisecond = static_cast<uint16_t>((unixNanos % 1000000000LLU) / 1000000LLU),
            .microsecond = static_cast<uint16_t>((unixNanos % 1000000LLU) / 1000LLU),
            .nanosecond = static_cast<uint16_t>(unixNanos % 1000LLU),
    };
}

DateTime GetCurrentUtcDateTime() noexcept {
    return TimeNanosToDateTime(CurrentTimeNanos(), 0);
}

DateTime GetCurrentLocalDateTime() noexcept {
    return TimeNanosToDateTime(CurrentTimeNanos(), GetUtcOffsetSeconds());

}

}
