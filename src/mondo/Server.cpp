//
// mondo/Server.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

//#include <vector>
//#include <autogen/mondo_grpc_pb.h>
//#include <util/ConfigUtil.h>
//#include <util/ThreadPool.h>
#include <util/NetUtil.h>
#include <util/TimeUtil.h>

#include "Server.h"

using namespace mondo;

constexpr size_t NUM_SERVER_THREADS = 4;
constexpr uint64_t MAX_NAP_DURATION = 1000; // msec

Server::Server(const ServerConfig& config)
        : _threads(NUM_SERVER_THREADS)
{
    int32_t port = config.port;
    if (port == 0) {
        port = NetUtil::find_available_port();
    } else {
        assert(NetUtil::port_is_valid(port));
    }
    _service = std::make_unique<Service>(port);
    _framePeriod = std::min(MAX_NAP_DURATION, config.framePeriod);
    start();
}

Server::~Server() {
    shutdown();
}

void Server::start() {
    if (!_service->isRunning()) {
        _threads.enqueue([&]() { _service->start(); });
    }
    if (!_running) {
        _running = true;
        _threads.enqueue([&]() { this->poll(); });
    }
}

void Server::shutdown() {
    if (_running) {
        _running = false;
        while (!_stopped) {
            // wait for poll() thread to stop
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

// use this to expose a Config to remote control
bool Server::registerConfig(ConfigUtil::ConfigInterface* config) {
    return false;
}

void Server::takeInput(uint64_t session_id, Blobs& blobs) {
}

void Server::giveOutput(uint64_t session_id, Blobs& blobs) {
}

uint64_t Server::getMsecToNextFrame() const {
    uint64_t now = TimeUtil::get_now_msec();
    return (now > _frameExpiry) ? 0 : _frameExpiry - now;
}

void Server::poll() {
    // call this on devoted thread
    _stopped = false;
    while (_running) {
        _frameExpiry = TimeUtil::get_now_msec() + _framePeriod;
        if (!_service->isRunning()) {
            _running = false;
            break;
        }

        // TODO: do polling work here

        uint64_t nap = std::min(MAX_NAP_DURATION, getMsecToNextFrame());
        if (nap > 0) {
            //TRACE_CONTEXT("nap", "Server,nap");
            std::this_thread::sleep_for(std::chrono::milliseconds(nap));
        }
    }
    if (_service->isRunning()) {
        _service->stop();
    }
    _stopped = true;
    _running = false;
}

