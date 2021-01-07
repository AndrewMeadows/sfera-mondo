//
// TraceUtil.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
//

#include "TraceUtil.h"

#include <sstream>
#include <fmt/format.h>

#include "LogUtil.h"
#include "TimeUtil.h"

using namespace TraceUtil;

std::unique_ptr<Tracer> Tracer::_instance;

// static
std::string Tracer::threadIdAsString() {
    // unfortunately, fmt doesn't know how to handle std::thread::id
    // hence this static helper method
    std::stringstream tid_str;
    tid_str << std::this_thread::get_id();
    return tid_str.str();
}

void Tracer::addEvent(const std::string& name, const std::string& cat, Phase ph) {
    if (_enabled) {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _events.push_back({name, cat, now(), std::this_thread::get_id(), -1, ph});
    }
}

void Tracer::addEventWithArgs(
        const std::string& name,
        const std::string& cat,
        Phase ph,
        const std::string& args) {
    if (_enabled) {
        std::lock_guard<std::mutex> lock(_eventMutex);
        int32_t args_index = (int32_t)(_args.size());
        _args.push_back(args);
        _events.push_back({name, cat, now(), std::this_thread::get_id(), args_index, ph});
    }
}

void Tracer::setCounter(
        const std::string& name,
        const std::string& cat,
        int64_t count)
{
    if (_enabled) {
        std::string args = fmt::format("\"{}\":{}", name, count);
        std::lock_guard<std::mutex> lock(_eventMutex);
        int32_t args_index = (int32_t)(_args.size());
        _args.push_back(args);
        _events.push_back({name, cat, now(), std::this_thread::get_id(), args_index, Phase::Counter});
    }
}

void Tracer::addMetaEvent(const std::string& type, const std::string& arg) {
    // Note: 'type' has a finite set of acceptable values
    //   process_name
    //   process_labels
    //   thread_name
    std::string arg_name;
    if (type == "process_name") {
        arg_name = "name";
    } else if (type == "process_labels") {
        arg_name = "labels";
    } else if (type == "thread_name") {
        arg_name = "name";
    }
    if (!arg_name.empty()) {
        // unfortunately, fmt doesn't know how to handle std::thread::id
        // so we format into string
        std::string tid_str = threadIdAsString();

        // meta_events get formatted to strings immediately
        std::lock_guard<std::mutex> lock(_eventMutex);
        _metaEvents.push_back(
            fmt::format(
                "{{\"name\":\"{}\",\"ph\":\"M\",\"pid\":1,\"tid\":{},\"args\":{{\"{}\":\"{}\"}}}}",
                type, tid_str, arg_name, arg));
    }
}

void Tracer::addMetaEvent(const std::string& type, uint32_t arg) {
    // Note: 'type' has a finite set of acceptable values
    //   process_sort_index
    //   thread_sort_index
    std::string arg_name;
    if (type == "process_sort_index" || type == "thread_sort_index") {
        std::string tid_str = threadIdAsString();

        // meta_events get formatted to strings immediately
        std::string event = fmt::format(
                "{{\"name\":\"{}\",\"M\",\"pid\":1,\"tid\":{},\"args\":{{\"sort_index\":{}}}}}",
                type, tid_str, arg);
        std::lock_guard<std::mutex> lock(_eventMutex);
        _metaEvents.push_back(event);
    }
}

void Tracer::advanceConsumers() {
    if (_events.empty()) {
        return;
    }

    // swap events out
    std::vector<Event> events;
    std::vector<std::string> args;
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        events.swap(_events);
        args.swap(_args);
    }

    if (_consumers.empty()) {
        return;
    }

    // convert events to strings
    std::vector<std::string> event_strings;
    std::string ph_str("a");
    for (size_t i = 0; i < events.size(); ++i) {
        const auto& event = events[i];
        // for speed we use fmt formatting where possible...
        ph_str[0] = event.ph;
        std::stringstream stream;
        stream << fmt::format(
                "{{\"name\":\"{}\",\"cat\":\"{}\",\"ph\":\"{}\",\"ts\":{},\"pid\":1",
                event.name, event.cat, ph_str, event.ts);
        // and std::ostream formatting when necessary...
        stream << ",\"tid\":" << event.tid;
        if (event.args_index != -1) {
            stream << fmt::format(",\"args\":{}", args[event.args_index]);
        }
        stream << "}";
        event_strings.push_back(stream.str());
    }

    // consume event strings
    uint64_t now = TimeUtil::get_now_msec();
    std::lock_guard<std::mutex> lock(_consumerMutex);
    std::vector<Tracer::Consumer*> expired_consumers;
    size_t i = 0;
    while (i < _consumers.size()) {
        Tracer::Consumer* consumer = _consumers[i];
        consumer->consumeEvents(event_strings);
        consumer->checkExpiry(now);
        if (consumer->isExpired()) {
            expired_consumers.push_back(consumer);
            size_t last_idx = _consumers.size() - 1;
            if (i != last_idx) {
                _consumers[i] = _consumers[last_idx];
            } else {
                ++i;
            }
            _consumers.pop_back();
            if (_consumers.size() == 0) {
                _enabled = false;
                LOG1("trace enabled={}\n", false);
            }
            continue;
        }
        ++i;
    }

    // complete expired consumers with meta_events
    if (expired_consumers.size() > 0) {
        // copy meta_events under lock
        std::vector<std::string> meta_events;
        {
            // Note: we're locking _eventMutex under _consumerMutex
            // which means we must never lock them in reverse order elsewhere
            // or risk deadlock.
            std::lock_guard<std::mutex> lock(_eventMutex);
            meta_events = _metaEvents;
        }
        // feed meta_events to consumers
        for (size_t i = 0; i < expired_consumers.size(); ++i) {
            Tracer::Consumer* consumer = expired_consumers[i];
            consumer->finish(meta_events);
        }
    }
}

// call this for clean shutdown of active consumers
void Tracer::shutdown() {
    {
        std::lock_guard<std::mutex> lock(_consumerMutex);
        for (size_t i = 0; i < _consumers.size(); ++i) {
            _consumers[i]->updateExpiry(0);
        }
    }
    advanceConsumers();
}

void Tracer::addConsumer(Tracer::Consumer* consumer) {
    if (consumer) {
        consumer->updateExpiry(TimeUtil::get_now_msec());
        std::lock_guard<std::mutex> lock(_consumerMutex);
        _consumers.push_back(consumer);
        if (!_enabled.load()) {
            _enabled = true;
            LOG1("trace enabled={}\n", _enabled);
        }
    }
}

void Tracer::removeConsumer(Tracer::Consumer* consumer) {
    // Note: no need to call this unless you're closing the consumer early
    // (e.g. before isComplete)
    std::lock_guard<std::mutex> lock(_consumerMutex);
    size_t i = 0;
    while (i < _consumers.size()) {
        if (consumer == _consumers[i]) {
            size_t last_idx = _consumers.size() - 1;
            if (i != last_idx) {
                _consumers[i] = _consumers[last_idx];
            } else {
                ++i;
            }
            _consumers.pop_back();
            continue;
        }
        ++i;
    }
    if (_consumers.size() == 0) {
        _enabled = false;
        LOG1("trace enabled={}\n", _enabled);
    }
}

TraceToFile::TraceToFile(uint64_t lifetime, const std::string& filename)
    : Tracer::Consumer(lifetime), _file(filename)
{
    _stream.open(_file);
    if (!_stream.is_open()) {
        LOG("failed to open trace file='{}'\n", _file);
        _file.clear();
    } else {
        LOG1("opened trace='{}'\n", _file);
        _stream << "{\"traceEvents\":[\n";
    }
}

void TraceToFile::consumeEvents(const std::vector<std::string>& events) {
    if (_stream.is_open()) {
        for (const auto& event : events) {
            _stream << event << ",\n";
        }
    }
}

void TraceToFile::finish(const std::vector<std::string>& meta_events) {
    Tracer::Consumer::finish(meta_events);
    if (_stream.is_open()) {
        // TRICK: end with bogus "complete" event sans ending comma
        // (this simplifies consume_event() logic)
        std::string tid_str = TraceUtil::Tracer::threadIdAsString();
        uint64_t ts = TraceUtil::Tracer::instance().now();
        std::string bogus_event = fmt::format(
            "{{\"name\":\"end_of_trace\",\"ph\":\"X\",\"pid\":1,\"tid\":{},\"ts\":{},\"dur\":1000}}",
            tid_str, ts);
        _stream << bogus_event << "\n]\n}\n"; // close array instead of comma

        _stream.close();
        LOG1("closed trace='{}'\n", _file);
    }
}
