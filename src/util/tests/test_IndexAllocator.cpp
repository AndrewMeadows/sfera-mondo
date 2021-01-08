//
// test_IndexAllocator.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <util/IndexAllocator.h>

TEST(IndexAllocator_test, allocate_free_clear_32) {
    using Index_t = int32_t;
    constexpr Index_t NUM_INDICES = 100;
    IndexAllocator<Index_t> allocator(NUM_INDICES);
    EXPECT_EQ(0, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(0, allocator.getNumAllocated());

    // fill the allocator
    for (Index_t i = 0; i < NUM_INDICES; ++i) {
        Index_t index = allocator.allocate();
        EXPECT_EQ(i, index);
        EXPECT_EQ(i + 1, allocator.getNumLive());
        EXPECT_EQ(0, allocator.getNumFree());
        EXPECT_EQ(i + 1, allocator.getNumAllocated());
    }

    // try to allocate over the max
    Index_t failed_index = allocator.allocate();
    EXPECT_EQ(-1, failed_index);
    EXPECT_EQ(NUM_INDICES, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

    // free one get one
    {
        constexpr Index_t SOME_INDEX = 37;
        allocator.free(SOME_INDEX);
        EXPECT_EQ(NUM_INDICES - 1, allocator.getNumLive());
        EXPECT_EQ(1, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

        Index_t i = allocator.allocate();
        EXPECT_EQ(SOME_INDEX, i);
        EXPECT_EQ(NUM_INDICES, allocator.getNumLive());
        EXPECT_EQ(0, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());
    }

    // free several get one
    {
        const Index_t indices[] = { 24, 57, 35, 17, 44, 96 };
        constexpr Index_t NUM_INDICES_TO_FREE = 6;
        Index_t lowest_freed = NUM_INDICES;
        for (Index_t i = 0; i < NUM_INDICES_TO_FREE; ++i) {
            allocator.free(indices[i]);
            if (indices[i] < lowest_freed) {
                lowest_freed = indices[i];
            }
        }
        EXPECT_EQ(NUM_INDICES - NUM_INDICES_TO_FREE, allocator.getNumLive());
        EXPECT_EQ(NUM_INDICES_TO_FREE, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

        // get one (should be lowest available index)
        Index_t new_index = allocator.allocate();
        EXPECT_EQ(lowest_freed, new_index);
        EXPECT_EQ(NUM_INDICES - NUM_INDICES_TO_FREE + 1, allocator.getNumLive());
        EXPECT_EQ(NUM_INDICES_TO_FREE - 1, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());
    }

    // clear
    allocator.clear();
    EXPECT_EQ(0, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(0, allocator.getNumAllocated());
}

TEST(IndexAllocator_test, allocate_free_clear_16) {
    using Index_t = int16_t;
    constexpr Index_t NUM_INDICES = 100;
    IndexAllocator<Index_t> allocator(NUM_INDICES);
    EXPECT_EQ(0, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(0, allocator.getNumAllocated());

    // fill the allocator
    for (Index_t i = 0; i < NUM_INDICES; ++i) {
        Index_t index = allocator.allocate();
        EXPECT_EQ(i, index);
        EXPECT_EQ(i + 1, allocator.getNumLive());
        EXPECT_EQ(0, allocator.getNumFree());
        EXPECT_EQ(i + 1, allocator.getNumAllocated());
    }

    // try to allocate over the max
    Index_t failed_index = allocator.allocate();
    EXPECT_EQ(-1, failed_index);
    EXPECT_EQ(NUM_INDICES, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

    // free one get one
    {
        constexpr Index_t SOME_INDEX = 37;
        allocator.free(SOME_INDEX);
        EXPECT_EQ(NUM_INDICES - 1, allocator.getNumLive());
        EXPECT_EQ(1, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

        Index_t i = allocator.allocate();
        EXPECT_EQ(SOME_INDEX, i);
        EXPECT_EQ(NUM_INDICES, allocator.getNumLive());
        EXPECT_EQ(0, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());
    }

    // free several get one
    {
        const Index_t indices[] = { 24, 57, 35, 17, 44, 96 };
        constexpr Index_t NUM_INDICES_TO_FREE = 6;
        Index_t lowest_freed = NUM_INDICES;
        for (Index_t i = 0; i < NUM_INDICES_TO_FREE; ++i) {
            allocator.free(indices[i]);
            if (indices[i] < lowest_freed) {
                lowest_freed = indices[i];
            }
        }
        EXPECT_EQ(NUM_INDICES - NUM_INDICES_TO_FREE, allocator.getNumLive());
        EXPECT_EQ(NUM_INDICES_TO_FREE, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());

        // get one (should be lowest available index)
        Index_t new_index = allocator.allocate();
        EXPECT_EQ(lowest_freed, new_index);
        EXPECT_EQ(NUM_INDICES - NUM_INDICES_TO_FREE + 1, allocator.getNumLive());
        EXPECT_EQ(NUM_INDICES_TO_FREE - 1, allocator.getNumFree());
        EXPECT_EQ(NUM_INDICES, allocator.getNumAllocated());
    }

    // clear
    allocator.clear();
    EXPECT_EQ(0, allocator.getNumLive());
    EXPECT_EQ(0, allocator.getNumFree());
    EXPECT_EQ(0, allocator.getNumAllocated());
}

int main(int32_t argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
