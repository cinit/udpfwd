//
// Created by sulfate on 8/31/23.
//

#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <cstdint>

namespace platform::time {

uint64_t CurrentTimeMillis() noexcept;

uint64_t CurrentTimeNanos() noexcept;

uint64_t CurrentTimeSeconds() noexcept;

struct DateTime {
    uint16_t year = 1970;
    uint8_t month = 1; // starts from 1
    uint8_t day = 1; // starts from 1
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint16_t millisecond = 0;
    uint16_t microsecond = 0;
    uint16_t nanosecond = 0;
};

DateTime TimeSecondsToDateTime(uint64_t unixSeconds, int64_t utcOffsetSeconds) noexcept;

DateTime TimeMillisToDateTime(uint64_t unixMillis, int64_t utcOffsetSeconds) noexcept;

DateTime TimeNanosToDateTime(uint64_t unixNanos, int64_t utcOffsetSeconds) noexcept;

DateTime GetCurrentUtcDateTime() noexcept;

DateTime GetCurrentLocalDateTime() noexcept;

int64_t GetUtcOffsetSeconds() noexcept;

}

#endif //UTIL_TIME_H
