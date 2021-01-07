//
// LogUtil.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LogUtil.h"

uint32_t g_log_verbosity = 0;

void LogUtil::set_verbosity(uint32_t verbosity) {
    g_log_verbosity = verbosity;
}

uint32_t LogUtil::get_verbosity() {
    return g_log_verbosity;
}
