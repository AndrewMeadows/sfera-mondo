//
// mondo/Service.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Service.h"

#include <thread>
#include <chrono>
#include <string>

#include <fmt/format.h>

using namespace mondo;

Service::Service(int32_t port) : _grpcServicePort(port) {
    grpc::ServerBuilder builder;

    // listen without any authentication mechanism
    std::string uri = fmt::format("[::]:{}", _grpcServicePort);
    builder.AddListeningPort(uri, grpc::InsecureServerCredentials());

    // register ourselves as "synchronous" Service
    builder.RegisterService(this);

    // assemble the server
    _grpcServer = builder.BuildAndStart();
}

// call start() on devoted thread
void Service::start() {
    if (!_running) {
        _running = true;
        _stopped = false;
        // process requests until Shutdown
        _grpcServer->Wait();
    }
    _stopped = true;
}

// stop() will block until threaded work is done
void Service::stop() {
    if (_running) {
        _running = false;
        _grpcServer->Shutdown();
    }
    // wait until doThreadedWork() is done
    while (!_stopped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// rpc StartSession (LoginRequest) returns (Input) {}
grpc::Status Service::StartSession(
        grpc::ServerContext* context,
        const LoginRequest* request,
        Input* reply)
{
    uint64_t sessionId = 0;
    if (_sessionManager) {
        sessionId = _sessionManager->getOrAddSessionId(request->user(), request->password());
    }
    reply->set_secret(sessionId);
    if (sessionId > 0) {
        if (_dataExchange) {
            _dataExchange->registerId(sessionId);
            // TODO: figure out how to do this more efficiently with less copying
            _dataExchange->showData(sessionId, &(request->blobs()));
            const Data* replyData = _dataExchange->borrowData(sessionId);
            if (replyData) {
                Data::const_iterator itr = replyData->begin();
                while (itr != replyData->end()) {
                    Blob* newBlob = reply->add_blobs();
                    newBlob->set_type(itr->type());
                    newBlob->set_msg(itr->msg());
                    ++itr;
                }
                _dataExchange->endBorrow(replyData);
            }
        }
    }
    return grpc::Status::OK;
}

// rpc PollInOut (Input) returns (Output) {}
grpc::Status Service::PollInOut(
        grpc::ServerContext* context,
        const Input* request,
        Output* reply)
{
    uint64_t sessionId = request->secret();
    bool session_is_valid = _sessionManager && _sessionManager->isValid(sessionId);
    if (session_is_valid) {
        if (_dataExchange) {
            _dataExchange->showData(sessionId, &(request->blobs()));
            const Data* replyData = _dataExchange->borrowData(sessionId);
            if (replyData) {
                Data::const_iterator itr = replyData->begin();
                while (itr != replyData->end()) {
                    Blob* newBlob = reply->add_blobs();
                    newBlob->set_type(itr->type());
                    newBlob->set_msg(itr->msg());
                    ++itr;
                }
                _dataExchange->endBorrow(replyData);
            }
        }
    }
    reply->set_success(session_is_valid);
    return grpc::Status::OK;
}

/*
// rpc StreamIn (stream Input) returns (Output) {}
grpc::Status Service::StreamIn(
        grpc::ServerContext* context,
        const Input* request,
        Output* reply)
{
    return grpc::Status::OK;
}

// rpc StreamOut (Input) returns (stream Output) {}
grpc::Status Service::StreamOut(
        grpc::ServerContext* context,
        const Input* request,
        Output* reply)
{
    return grpc::Status::OK;
}

// rpc StreamInOut (stream Input) returns (stream Output) {}
grpc::Status Service::StreamInOut(
        grpc::ServerContext* context,
        const Input* request,
        Output* reply)
{
    return grpc::Status::OK;
}
        */
