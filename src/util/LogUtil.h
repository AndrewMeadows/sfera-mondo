//
// LogUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <fmt/format.h>

#include "TimeUtil.h"

namespace LogUtil {

void set_verbosity(uint32_t verbosity);
uint32_t get_verbosity();

} // namespace LogUtil

// compile-time options
#define LOGUTIL_ENABLE_TIMESTAMP
#define LOGUTIL_ENABLE_VERBOSITY

// most compilers will optimize this NOOP out
#define LOGUTIL_NOOP do{}while(0)

#ifdef LOGUTIL_ENABLE_TIMESTAMP
    // format and print with fmt
    #define LOGUTIL_TIMESTAMP fmt::print("{} ", TimeUtil::get_local_datetime_string_with_msec())
    #define LOGUTIL_TIMESTAMP_LEVEL(X) fmt::print("{} ({}) ", TimeUtil::get_local_datetime_string_with_msec(), X)
#else
    #define LOGUTIL_TIMESTAMP LOGUTIL_NOOP
    #define LOGUTIL_TIMESTAMP_LEVEL(X) LOGUTIL_NOOP
#endif

// NOTE: LOG is a macro and has the following quirks:
// (1) It must be used in a single line.  For multi-line instances
//     break the line using EOL backslashes.
// (2) It must have at least one variable argument.
//     You can't do: LOG("foo").
//     At a minimum you must do: LOG("foo {}", bar)
#define LOG(fmtstr, ...) LOGUTIL_TIMESTAMP;fmt::print(fmtstr, __VA_ARGS__)

#ifdef LOGUTIL_ENABLE_VERBOSITY
    #define LOG1(fmtstr, ...) if(LogUtil::get_verbosity()>0){LOGUTIL_TIMESTAMP_LEVEL(1);fmt::print(fmtstr, __VA_ARGS__);}
    #define LOG2(fmtstr, ...) if(LogUtil::get_verbosity()>1){LOGUTIL_TIMESTAMP_LEVEL(2);fmt::print(fmtstr, __VA_ARGS__);}
    #define LOG3(fmtstr, ...) if(LogUtil::get_verbosity()>2){LOGUTIL_TIMESTAMP_LEVEL(3);fmt::print(fmtstr, __VA_ARGS__);}
#else
    #define LOG1(fmtstr, ...) LOGUTIL_NOOP
    #define LOG2(fmtstr, ...) LOGUTIL_NOOP
    #define LOG3(fmtstr, ...) LOGUTIL_NOOP
#endif

