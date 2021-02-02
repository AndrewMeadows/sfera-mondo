//
// RecentHistory.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <stdint.h>
#include <vector>

#include <glm/glm.hpp>

template < typename Event >
class RecentHistory {
public:

    class Consumer {
    public:
        uint32_t getVersion() const { return _version; }
        void setVersion(uint32_t version) { _version = version; }
        virtual void consumeEvent(const Event& event) = 0;
    private:
        uint32_t _version {0};
    };

    RecentHistory(uint32_t ring_capacity) {
        const uint32_t MAX_RING_SIZE = 512;
        if (ring_capacity > MAX_RING_SIZE) {
            ring_capacity = MAX_RING_SIZE;
        }
        _ringCapacity = ring_capacity;
        _events.resize(_ringCapacity);
    }

    uint32_t getSize() const {
        return (_head >= _tail) ?
            _head - _tail :
            _ringCapacity - (_tail - _head);
    }

    uint32_t getCapacity() const { return _ringCapacity; }

    void clearAndSetVersion(uint32_t version) {
        _head = 0;
        _tail = 0;
        _version = version;
    }

    uint32_t getVersion() const { return _version; }

    // copies event to history
    // use this if you must, else consider using swap() or take() when possible
    // (they should be faster)
    void copy(const Event& event) {
        _events[_head] = event;
        advanceHead();
    }

    void swap(Event& event) {
        _events[_head].swap(event);
        advanceHead();
    }

    // take() is slower than swap()
    // use it for clarity of code (if that is important)
    void take(Event& event) {
        _events[_head].clear();
        _events[_head].swap(event);
        advanceHead();
    }

    template<typename Consumer_t>
    bool advanceConsumer(Consumer_t& consumer) const {
        uint32_t version = consumer.getVersion();
        if (version > _version) {
            // wants a future version --> fail
            return false;
        }
        uint32_t snake_length = getSize();
        if (version < _version - snake_length) {
            // wants a version lost to history --> fail
            return false;
        }

        // compute idx where consumer will start
        uint32_t num_changes = _version - version;
        uint32_t idx = (num_changes > _head) ?
            (_ringCapacity + _head - num_changes) :
            (_head - num_changes);

        // read history
        while (idx != _head) {
            consumer.consumeEvent(_events[idx]);
            idx = (idx + 1) % _ringCapacity;
        }
        consumer.setVersion(_version);
        return true;
    }

protected:
    void advanceHead() {
        ++_version;
        _head = (_head + 1) % _ringCapacity;
        if (_head == _tail) {
            // snake has wrapped around on itself and is about to eat its tail
            // so we advance the tail
            // Note: any consumers not moving fast enough will lose data.
            // It is the duty of external logic to not push changes faster than
            // it advances its consumers.
            _tail = (_tail + 1) % _ringCapacity;
        }
    }

private:
    // recent changes are stored in a "snake" which moves around a "ring"
    // new changes are placed in front of the head
    // tail moves forward when necessary so as to not get eaten
    uint32_t _tail {0};
    uint32_t _head {0};
    uint32_t _ringCapacity {0};
    uint32_t _version {0};
    std::vector<Event> _events;
};
