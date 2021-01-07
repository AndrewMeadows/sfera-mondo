//
// RandomUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <glm/glm.hpp>

namespace RandomUtil {

    int32_t int32();

    uint32_t uint32();

    int64_t int64();

    // return from range [0, ES6::MAX_SAFE_INTEGER]
    uint64_t uint53();

    uint64_t uint64();

    // returns from range [0,1]
    float unitFloat();

    // returns from range [-1,1]
    float symmetricUnitFloat();

    // returns random point on surface of unit sphere
    glm::vec3 unitSphereSurface();

    // returns a random color as a uint32 0xRRGGBBAA
    // with user-set alpha in range [0,1]
    // and default to full opaque
    uint32_t color(float alpha = 1.0f);

} // RandomUtil namespace
