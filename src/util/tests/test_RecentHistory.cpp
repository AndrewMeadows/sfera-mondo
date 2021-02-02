//
// test_RecentHistory.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <vector>

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gtest/gtest.h>

#include <util/RandomUtil.h>
#include <util/RecentHistory.h>

// This test uses a series of rotations as "Events"

using Event = glm::quat;
using History = RecentHistory<Event>;

// helper
bool approximately_equal(const glm::quat& A, const glm::quat& B) {
    constexpr float DOT_PRODUCT_SLOP = 8.0e-7; // ~0.1 degree of error
    float dot_product = std::abs(glm::dot(A, B));
    fmt::print("adebug approximately_equal A=<{},{},{},{}> B=<{},{},{},{}> (1.0-dp)={}\n",
            A.w, A.x, A.y, A.z, B.w, B.x, B.y, B.z, std::abs(1.0f - dot_product));
    return std::abs(1.0f - dot_product) < DOT_PRODUCT_SLOP;
}

// UnitRotator is a Consumer which computes a final rotation
// of a sequence of untary local-frame rotations.
class UnitRotator : public History::Consumer {
public:
    UnitRotator() {}

    UnitRotator(const Event& event) : _rotation(event) {}

    const Event& getRotation() const { return _rotation; }

    void consumeEvent(const Event& event) override {
        // 'event' represents a delta_rotation in the local-frame
        // therefore it goes on the right side
        _rotation = glm::normalize(_rotation * event);
    }

    bool operator==(const UnitRotator& other) const {
        return approximately_equal(_rotation, other._rotation);
    }

    UnitRotator operator*(const UnitRotator& other) {
        return UnitRotator(_rotation * other._rotation);
    }

private:
    // glm::quat initialization sequence is: w,x,y,z
    Event _rotation { 1.0f, 0.0f, 0.0f, 0.0f };
};


TEST(RecentHistory_test, normal_operation) {
    // Consumers can consume events at different paces
    // but should always reach the same state in the end.

    uint32_t num_events = 31;
    // Note: for this test we need: ring_size > num_events
    uint32_t ring_size = num_events + 1;

    History history(ring_size);
    uint32_t expected_version = 0;
    EXPECT_EQ(expected_version, history.getVersion());

    // create some "events"
    // each rotation will have a fixed angle, but the axis will be random
    constexpr float ANGLE = 1.57f; // ~PI/2
    std::vector<Event> events;
    events.reserve(num_events);
    for (uint32_t i = 0; i < num_events; ++i) {
        glm::vec3 axis = RandomUtil::unitSphereSurface();
        Event event = glm::angleAxis(ANGLE, axis);
        events.push_back(event);
    }
    fmt::print("adebug wtf?\n");

    // precompute the accumulated rotations to use when verifying Consumer state
    std::vector<Event> accumulated_rotations;
    accumulated_rotations.reserve(num_events);
    glm::quat Q(1.0f, 0.0f, 0.0f, 0.0f);
    for (uint32_t i = 0; i < num_events; ++i) {
        Q = glm::normalize(Q * events[i]);
        accumulated_rotations.push_back(Q);
        fmt::print("adebug {} : <{}, {}, {}, {}>\n", i, Q.w, Q.x, Q.y, Q.z);
    }

    // add some (but not all) events to history
    uint32_t num_rotations = num_events / 3;
    uint32_t idx = 0;
    while (idx < num_rotations) {
        history.copy(events[idx]);
        ++expected_version;
        EXPECT_EQ(expected_version, history.getVersion());
        ++idx;
    }

    // create rotator_A and read history
    UnitRotator rotator_A;
    bool success = history.advanceConsumer(rotator_A);
    EXPECT_TRUE(success);

    // verfiy rotator_A has processed the events in the correct order
    EXPECT_EQ(rotator_A.getVersion(), history.getVersion());
    uint32_t qidx = rotator_A.getVersion() - 1;
    glm::quat Qa = rotator_A.getRotation();
    fmt::print("adebug qidx={}  version={}  Q=<{}, {}, {}, {}>\n", qidx, rotator_A.getVersion(), Qa.w, Qa.x, Qa.y, Qa.z);
    EXPECT_TRUE(approximately_equal(rotator_A.getRotation(), accumulated_rotations[qidx]));

    // add some more events to history
    num_rotations = (3 * num_events) / 4;
    while (idx < num_rotations) {
        history.copy(events[idx]);
        ++expected_version;
        ++idx;
    }
    EXPECT_EQ(expected_version, history.getVersion());

    // create a second consumer to read history
    UnitRotator rotator_B;
    success = history.advanceConsumer(rotator_B);
    EXPECT_TRUE(success);
    EXPECT_EQ(rotator_B.getVersion(), history.getVersion());
    qidx = rotator_B.getVersion() - 1;
    EXPECT_TRUE(approximately_equal(rotator_B.getRotation(), accumulated_rotations[qidx]));

    // create a third consumer to start after rotator_A stopped
    UnitRotator rotator_C;
    rotator_C.setVersion(rotator_A.getVersion());
    // ... and advance from there to end of history
    success = history.advanceConsumer(rotator_C);
    EXPECT_TRUE(success);
    EXPECT_EQ(rotator_C.getVersion(), history.getVersion());

    // verify B == A * C
    glm::quat A = rotator_A.getRotation();
    glm::quat B = rotator_B.getRotation();
    glm::quat C = rotator_C.getRotation();
    EXPECT_TRUE(approximately_equal(B, glm::normalize(A * C)));

    // finally add the rest of the objects
    while (idx < num_rotations) {
        history.copy(events[idx]);
        ++expected_version;
        ++idx;
        ++idx;
    }
    EXPECT_EQ(expected_version, history.getVersion());

    // advance all the consumers
    success = history.advanceConsumer(rotator_A);
    EXPECT_TRUE(success);
    EXPECT_EQ(rotator_A.getVersion(), history.getVersion());

    success = history.advanceConsumer(rotator_B);
    EXPECT_TRUE(success);
    EXPECT_EQ(rotator_B.getVersion(), history.getVersion());

    success = history.advanceConsumer(rotator_C);
    EXPECT_TRUE(success);
    EXPECT_EQ(rotator_C.getVersion(), history.getVersion());

    // verify A == B
    EXPECT_EQ(rotator_A, rotator_B);

    // verify B == oldA * C
    C = rotator_C.getRotation();
    EXPECT_TRUE(approximately_equal(B, glm::normalize(A * C)));
}

TEST(RecentHistory_test, lost_history) {
    // When Consumers are not advanced fast enough
    // the history's ring can "lose" events by overwritting them.
    // This test is to verify such lost history events are detected.

    uint32_t num_events = 31;
    // Note: for this test we need: ring_size < num_events
    uint32_t ring_size = 8;

    History history(ring_size);
    uint32_t expected_version = 0;

    // create some "events"
    // each rotation will have a fixed angle, but the axis will be random
    constexpr float ANGLE = 1.57f; // ~PI/2
    std::vector<Event> events;
    events.reserve(num_events);
    for (uint32_t i = 0; i < num_events; ++i) {
        glm::vec3 axis = RandomUtil::unitSphereSurface();
        Event event = glm::angleAxis(ANGLE, axis);
        events.push_back(event);
    }

    // precompute the accumulated rotations to use when verifying Consumer state
    std::vector<Event> accumulated_rotations;
    accumulated_rotations.reserve(num_events);
    glm::quat Q(1.0f, 0.0f, 0.0f, 0.0f);
    for (uint32_t i = 0; i < num_events; ++i) {
        Q = glm::normalize(Q * events[i]);
        accumulated_rotations.push_back(Q);
    }

    // add one event to history
    uint32_t idx = 0;
    history.copy(events[idx]);
    ++idx;

    // create two consumers and advance
    UnitRotator rotator_A, rotator_B;
    history.advanceConsumer(rotator_A);
    history.advanceConsumer(rotator_B);

    // fill the history ring
    uint32_t num_rotations = ring_size;
    while (idx < num_rotations) {
        history.copy(events[idx]);
        ++expected_version;
        ++idx;
    }

    // advance one of the consumers
    bool success = history.advanceConsumer(rotator_A);
    EXPECT_TRUE(success);

    // add two more events to history
    // which will start to overwrite the tail of the ring
    for (uint32_t i = 0; i < 2; ++i) {
        glm::vec3 axis = RandomUtil::unitSphereSurface();
        Event event = glm::angleAxis(ANGLE, axis);
        history.copy(event);
    }

    // first consumer should succeed
    success = history.advanceConsumer(rotator_A);
    EXPECT_TRUE(success);

    // second consumer should fail
    // because it is asking for lost history
    success = history.advanceConsumer(rotator_B);
    EXPECT_TRUE(!success);
}


int main(int32_t argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
