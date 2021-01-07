//
// TraceUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "TimeUtil.h"

namespace TraceUtil {

// The goal here is to provide a fast+simple trace tool rather than a
// complete one.  As a consequence not all Phase types are supported.
// Unsupported phases are those that require fields outside of this
// minimal subset:
//
//   name = human readable name for the event
//   cat = comma separated strings used for filtering
//   ph = phase type
//   ts = timestamp
//   tid = thread_id
//   pid = process_id
//   args = JSON string of special info
//
// For example, the 'Complete' event requires a 'dur' field and the
// 'Async*' events use 'id'.
//
enum Phase : char {
    // supported:
    DurationBegin = 'B',
    DurationEnd = 'E',
    Counter = 'C',
    Metadata = 'M',

    // unsupported:
    Complete = 'X',
    Instant = 'i',

    AsyncNestableStart = 'b',
    AsyncNestableInstant = 'n',
    AsyncNestableEnd = 'e',

    FlowStart = 's',
    FlowStep = 't',
    FlowEnd = 'f',

    Sample = 'P',

    ObjectCreated = 'N',
    ObjectSnapshot = 'O',
    ObjectDestroyed = 'D',

    MemoryDumpGlobal = 'V',
    MemoryDumpProcess = 'v',

    Mark = 'R',

    ClockSync = 'c',

    ContextEnter = '(',
    ContextLeave = ')'
};

// Note: Tracer is a singleton
class Tracer {
public:

    // To harvest trace events the pattern is:
    // (1) create a Consumer and give pointer to Tracer
    // (2) override consumeEvents() to do what you want with events
    // (3) when consumer is COMPLETE close and delete
    // (Tracer automatically removes consumer before COMPLETE)
    class Consumer {
    public:
        friend class Tracer;

        enum State : uint8_t {
            ACTIVE,  // collecting events
            EXPIRED, // lifetime is up
            COMPLETE // all done (has collected meta_events)
        };

        Consumer(uint64_t lifetime) {
            // Note: lifetime is limited because the chrome://tracing tool
            // can crash when browsing very large files
            constexpr uint64_t MAX_TRACE_CONSUMER_LIFETIME = 10 * TimeUtil::MSEC_PER_SECOND;
            if (lifetime > MAX_TRACE_CONSUMER_LIFETIME) {
                lifetime = MAX_TRACE_CONSUMER_LIFETIME;
            }
            _lifetime = lifetime;
        }

        // override this pure virtual method to Do Stuff with events
        // each event will be a JSON string as per the google tracing API
        virtual void consumeEvents(const std::vector<std::string>& events) = 0;

        // called by Tracer on add
        // but can also be used to change expiry on the fly
        void updateExpiry(uint64_t now) { _expiry = now + _lifetime; }

        bool isExpired() const { return _state == State::EXPIRED; };
        bool isComplete() const { return _state == State::COMPLETE; }

        // called by Trace after consumeEvents
        void checkExpiry(uint64_t now) {
            if (now > _expiry) {
                _state = EXPIRED;
            }
        }

        // called by Tracer after expired
        virtual void finish(const std::vector<std::string>& meta_events) {
            assert(_state == State::EXPIRED);
            consumeEvents(meta_events);
            _state = State::COMPLETE;
        }

    protected:
        State _state { State::ACTIVE };
        uint64_t _lifetime; // msec
        uint64_t _expiry { TimeUtil::DISTANT_FUTURE };
    };

    static Tracer& instance() {
        static std::unique_ptr<Tracer> _instance(new Tracer());
        return (*_instance);
    }

    // helper
    static std::string threadIdAsString();

    Tracer() : _startTime(std::chrono::high_resolution_clock::now()) { }

    uint64_t now() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - _startTime).count();
    }

    void addEvent(const std::string& name, const std::string& cat, Phase ph);
    void addEventWithArgs(
            const std::string& name,
            const std::string& cat,
            Phase ph,
            const std::string& args);
    void setCounter(const std::string& name, const std::string& cat, int64_t count);

    // type = process_name, process_lables, or thread_name
    void addMetaEvent(const std::string& type, const std::string& arg);

    // type = process_sort_index, or thread_sort_index
    void addMetaEvent(const std::string& type, uint32_t arg);

    void addConsumer(Consumer* consumer);
    void advanceConsumers();
    void shutdown();

    // don't call removeConsumer() unless you know what you're doing
    // (e.g. shutting down before consumers are complete)
    void removeConsumer(Consumer* consumer);

private:
    static std::unique_ptr<Tracer> _instance;
    mutable std::mutex _eventMutex;
    mutable std::mutex _consumerMutex;
    std::chrono::high_resolution_clock::time_point _startTime;

    // Note: Event does not store 'pid' because we assume
    // all events are for the same process.
    struct Event {
        std::string name;
        std::string cat;
        uint64_t ts;
        std::thread::id tid;
        int32_t args_index;
        Phase ph;
    };

    std::vector<Consumer*> _consumers;

    std::vector<Event> _events;
    std::vector<std::string> _metaEvents;
    std::vector<std::string> _args;
    std::atomic<bool> _enabled{ false };

    Tracer(Tracer const&); // Don't Implement
    void operator=(Tracer const&); // Don't implement
};

// TraceContext creates a DurationBegin event in ctor
// and an DurationEnd event in dtor
class TraceContext {
public:
    TraceContext(const std::string& name, const std::string& cat)
        : _name(name), _cat(cat)
    {
        Tracer::instance().addEvent(_name, _cat, Phase::DurationBegin);
    }
    ~TraceContext() {
        Tracer::instance().addEvent(_name, _cat, Phase::DurationEnd);
    }
    std::string _name;
    std::string _cat;
};

// TraceToFile is a simple consumer for saving events to file
class TraceToFile : public Tracer::Consumer {
public:
    TraceToFile(uint64_t lifetime, const std::string& filename);
    void consumeEvents(const std::vector<std::string>& events) final override;
    void finish(const std::vector<std::string>& meta_events) final override;
    bool isOpen() const { return _stream.is_open(); }
private:
    std::string _file;
    std::ofstream _stream;
};
} // namespace TraceUtil
