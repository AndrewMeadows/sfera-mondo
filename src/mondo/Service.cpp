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

using namespace mondo:

Service(int32_t port) : _port(port) {
    grpc::ServerBuilder builder;

    // listen without any authentication mechanism
    std::string uri = fmt::format("[::]:{}", _port);
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

int32_t getPort() const { return _grpcServicePort; }
bool isRunning() const { return _running; }

// rpc StartSession (LoginRequest) returns (Input) {}
grpc::Status Service::StartSession(
        grpc::ServerContext* context,
        const mondo::LoginRequest* request,
        mondo::Input* reply)
{
    return grpc::Status::OK;
}

// rpc PollInOut (Input) returns (Output) {}
grpc::Status Service::PollInOut(
        grpc::ServerContext* context,
        const mondo::Input* request,
        mondo::Ouput* reply)
{
    return grpc::Status::OK;
}

/*
// rpc StreamIn (stream Input) returns (Output) {}
grpc::Status Service::StreamIn(
        grpc::ServerContext* context,
        const mondo::Input* request,
        mondo::Ouput* reply)
{
    return grpc::Status::OK;
}

// rpc StreamOut (Input) returns (stream Output) {}
grpc::Status Service::StreamOut(
        grpc::ServerContext* context,
        const mondo::Input* request,
        mondo::Ouput* reply)
{
    return grpc::Status::OK;
}

// rpc StreamInOut (stream Input) returns (stream Output) {}
grpc::Status Service::StreamInOut(
        grpc::ServerContext* context,
        const mondo::Input* request,
        mondo::Ouput* reply)
{
    return grpc::Status::OK;
}
        */
