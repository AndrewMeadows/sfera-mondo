//
// TimeUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
# pragma once

#include <chrono>
#include <ctime>
#include <string>

#include <fmt/format.h>

namespace TimeUtil {

constexpr uint64_t DISTANT_FUTURE = uint64_t(-1);
constexpr uint64_t MSEC_PER_SECOND = uint64_t(1e3);
constexpr uint64_t USEC_PER_SECOND = uint64_t(1e6);
constexpr uint64_t USEC_PER_MSEC = uint64_t(1e3);
constexpr uint64_t MSEC_PER_MINUTE = 60 * MSEC_PER_SECOND;
constexpr uint64_t MSEC_PER_HOUR = 60 * MSEC_PER_MINUTE;
constexpr uint64_t MSEC_PER_DAY = 24 * MSEC_PER_HOUR;
constexpr uint64_t MSEC_PER_YEAR = 365 * MSEC_PER_DAY;

inline uint64_t get_now_usec() {
    using namespace std::chrono;
    static uint64_t usec_offset = 0;
    if (usec_offset == 0) {
        usec_offset = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count()
            - duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    }
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count() + usec_offset;
}

inline uint64_t get_now_msec() {
    using namespace std::chrono;
    static uint64_t msec_offset = 0;
    if (msec_offset == 0) {
        msec_offset = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()
            - duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() + msec_offset;
}

// YYYYMMDD_HH:MM:SS
std::string get_local_datetime_string(uint64_t now_msec);
std::string get_local_datetime_string();

// YYYYMMDD_HH:MM:SS.msec
std::string get_local_datetime_string_with_msec(uint64_t now_msec);
std::string get_local_datetime_string_with_msec();

} // namespace TimeUtil

