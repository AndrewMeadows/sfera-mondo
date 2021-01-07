//
// RandomUtil.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RandomUtil.h"

#include <algorithm>
#include <chrono>
#include <math.h>
#include <random>

// random_engine with a unique seed
std::default_random_engine RandomUtil_engine(
        (uint32_t) std::chrono::system_clock::now().time_since_epoch().count());

// distributions
std::uniform_int_distribution<int32_t> RandomUtil_dis_int32(INT32_MIN, INT32_MAX);
std::uniform_int_distribution<uint32_t> RandomUtil_dis_uint32(0, UINT32_MAX);
std::uniform_int_distribution<int64_t> RandomUtil_dis_int64(INT64_MIN, INT64_MAX);
std::uniform_int_distribution<uint64_t> RandomUtil_dis_uint64(0, UINT64_MAX);
std::uniform_real_distribution<> RandomUtil_dis_unit_float(0.0f, 1.0f);

constexpr uint64_t ES6_MAX_SAFE_INTEGER = 9007199254740991; // 2^53 - 1
std::uniform_int_distribution<uint64_t> RandomUtil_dis_uint53(0, ES6_MAX_SAFE_INTEGER);

int32_t RandomUtil::int32() {
    return RandomUtil_dis_int32(RandomUtil_engine);
}

uint32_t RandomUtil::uint32() {
    return RandomUtil_dis_uint32(RandomUtil_engine);
}

int64_t RandomUtil::int64() {
    return RandomUtil_dis_int64(RandomUtil_engine);
}

// return from range [0, ES6::MAX_SAFE_INTEGER]
uint64_t RandomUtil::uint53() {
    return RandomUtil_dis_uint53(RandomUtil_engine);
}

uint64_t RandomUtil::uint64() {
    return RandomUtil_dis_uint64(RandomUtil_engine);
}

// returns from range [0,1]
float RandomUtil::unitFloat() {
    return (float)RandomUtil_dis_unit_float(RandomUtil_engine);
}

// returns from range [-1,1]
float RandomUtil::symmetricUnitFloat() {
    return 2.0f * (float)RandomUtil_dis_unit_float(RandomUtil_engine) - 1.0f;
}

// returns random point on surface of unit sphere
glm::vec3 RandomUtil::unitSphereSurface() {
    float z = RandomUtil::symmetricUnitFloat();
    float angle = 3.141592653589793f * symmetricUnitFloat();
    float c = sqrt(1.0f - z*z);
    return glm::vec3(c * sin(angle), c * cos(angle), z);
}

// returns a random color as a uint32 0xRRGGBBAA
// with user-set alpha in range [0,1]
// and default to full opaque
uint32_t RandomUtil::color(float alpha) {
    uint32_t alpha_byte =
        (uint32_t)(std::floor(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
    return (uint32() & 0xffffff00) + alpha_byte;
}
