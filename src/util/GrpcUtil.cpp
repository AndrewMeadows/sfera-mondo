//
// GrpcUtil.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "GrpcUtil.h"

#include <thread>

#include "TraceMacros.h"

using namespace GrpcUtil;

Client::Client(const std::string& uri)
    :   _queue(std::make_unique<grpc::CompletionQueue>()),
        _channel(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())),
        _uri(uri)
{
}

Client::~Client() {
}

void Client::addCall(GrpcUtil::Call* call) {
    // the call will cast the stub to the right type
    call->start(_queue.get(), _stub);
}

void Client::start() {
    if (_running) {
        return;
    }
    TRACE_THREAD("Client");
    _running = true;
    _stopped = false;
    while (_running) {
        void* tag;
        bool read_ok = false;

        // Block until the next result is available from the queue.
        // The return value of Next() should always be checked.  It
        // tells us whether there is an event or the queue is shutting down.
        // The 'read_ok' is true if the event was successfully read, else false
        // (in particular: it is false when a stream closes).
        bool event = _queue->Next(&tag, &read_ok);
        if (!event) {
            // shutting down
            _running = false;
            break;
        }

        // The tag is always a pointer to a Call which has a processReply() method
        GrpcUtil::Call* call = static_cast<GrpcUtil::Call*>(tag);
        {
            TRACE_CONTEXT("processReply", "GrpcUtil::Client");
            call->processReply(read_ok);
        }

        // the destroy() here is to signal "the queue no longer cares" about the call
        if (!call->keepAlive()) {
            call->destroy();
        }
    }
    _queue->Shutdown();

    // drain queue before delete, else will assert on delete
    void* ignored_tag;
    bool ignored_ok;
    while (_queue->Next(&ignored_tag, &ignored_ok)) { }
    _stopped = true;
}

void Client::stop() {
    _queue->Shutdown();
    _running = false;
    while (!_stopped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Client::setStub(void* stub) {
    _stub = stub;
}



//#include <fmt/format.h>
//#include <string>


AsynchServer::~AsynchServer() {
    // drain queue before delete, else will assert
    void* ignored_tag;
    bool ignored_ok;
    while (_queue->Next(&ignored_tag, &ignored_ok)) { }
}

void AsynchServer::buildService(uint32_t port) {
    _port = port;
    grpc::ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    std::string uri = fmt::format("[::]:{}", port);
    builder.AddListeningPort(uri, grpc::InsecureServerCredentials());

    registerService(builder);

    // Obtain a pointer to the completion queue used for asynchronous
    // communication with the gRPC runtime.
    _queue = builder.AddCompletionQueue();

    // assemble the server.
    _grpcServer = builder.BuildAndStart();
}

void AsynchServer::start() {
    if (_running) {
        return;
    }
    spawnHandlers();

    void* tag;  // unique value identifying the event
    bool ok;
    _running = true;
    while (_running) {
        // Wait for next event from the queue.
        // The event is uniquely identified by its tag.
        // The return value of Next should always be checked.
        // It tells us whether it is an event (true)
        // or the queue is shutting down (false);
        bool event = _queue->Next(&tag, &ok);
        if (!event || !ok) {
            // shutting down
            _running = false;
            break;
        }
        // the "tag" is always a void-pointer to an GrpcUtil::Handler
        // and we always call proceed() on it to advance it through
        // its short state machine
        static_cast<GrpcUtil::Handler*>(tag)->proceed();
    }

    // drain queue else will assert on delete
    void* ignored_tag;
    bool ignored_ok;
    while (_queue->Next(&ignored_tag, &ignored_ok)) { }
    _stopped = true;
}

void AsynchServer::stop() {
    _running = false;
    // always shutdown grpc_server BEFORE queue
    _grpcServer->Shutdown();
    _queue->Shutdown();
}

