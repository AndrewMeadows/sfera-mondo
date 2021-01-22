//
// TraceMacros.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
# pragma once

#include "TraceUtil.h"

#define USE_TRACE_UTIL
#ifdef USE_TRACE_UTIL

    // Usage pattern:
    //
    // (1) Add TRACE_CONTEXT where you want to measure timing.
    //     Only one per scope, but you can add to embedded scopes.
    //
    // (2) Implement a custom Consumer class to handle trace events
    //     or use TraceUtil::WriteToFile.
    //
    // (3) Add Consumer to Tracer. This will enable trace events.
    //
    // (4) Every so often (e.g. inside mainloop) do the following:
    //
    //   (A) call TRACE_ADVANCE_CONSUMERS
    //
    //   (B) when Consumer->isComplete() --> delete it
    //        (it will have been automatically removed from Tracer)
    //
    // (5) When Tracer has no Consumers it stops collecting trace
    //     events.
    //
    // (6) Add TRACE_SHUTDOWN after mainloop exits in case there
    //     are unfinished Consumers on shutdown.
    //

    #define TRACE_ADVANCE_CONSUMERS ::TraceUtil::Tracer::instance().advanceConsumers()
    #define TRACE_SHUTDOWN ::TraceUtil::Tracer::instance().shutdown()

    // use these to add meta_events
    #define TRACE_PROCESS(name) ::TraceUtil::Tracer::instance().addMetaEvent("process_name", name)
    #define TRACE_THREAD(name) ::TraceUtil::Tracer::instance().addMetaEvent("thread_name", name)
    #define TRACE_THREAD_SORT(idx) ::TraceUtil::Tracer::instance().addMetaEvent("thread_sort_index", idx)

    // use TRACE_CONTEXT for easy Duration events
    #define TRACE_CONTEXT(name, cat) ::TraceUtil::Context trace_context(name, cat)

    // use TRACE_BEGIN/END only if you know what you're doing
    // and TRACE_CONTEXT doesn't work for you
    #define TRACE_BEGIN(name, cat) ::TraceUtil::Tracer::instance().addEvent(name, cat, ::TraceUtil::Phase::DurationBegin)
    #define TRACE_END(name, cat) ::TraceUtil::Tracer::instance().addEvent(name, cat, ::TraceUtil::Phase::DurationBegin)

#else
    // all macros are no-ops
    //
    // we use 'do{}while(0)' as the no-op because:
    //
    // (1) most compilers will optimize it out
    // (2) it is a single expression and should not unexpectedly break contexts
    //     (e.g. when used in a single-line 'if' context without brackets)

    #define TRACE_ADVANCE_CONSUMERS do{}while(0)
    #define TRACE_SHUTDOWN do{}while(0)

    #define TRACE_PROCESS(name) do{}while(0)
    #define TRACE_THREAD(name) do{}while(0)
    #define TRACE_THREAD_SORT(idx) do{}while(0)

    #define TRACE_CONTEXT(name, cat) do{}while(0)
    #define TRACE_BEGIN(name, cat) do{}while(0)
    #define TRACE_END(name, cat) do{}while(0)

#endif // USE_TRACE_UTIL
