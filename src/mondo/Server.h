//
// mondo/Server.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
# pragma once

#include <vector>
#include <memory>

#include <autogen/mondo.pb.h>
#include <util/ConfigUtil.h>
#include <util/ThreadPool.h>

#include "Service.h"

namespace mondo {

using Blobs = std::vector<Blob>;

constexpr uint64_t DEFAULT_SERVER_FRAME_PERIOD = 20; // msec

struct ServerConfig {
    uint64_t framePeriod { DEFAULT_SERVER_FRAME_PERIOD };
    int32_t port { 0 };
};


// Server has a Service and relays Blobs.
class Server {
public:
    Server(const ServerConfig& config);
    ~Server();

    // non-blocking
    void start();
    void stop() { _running = false; }

    // blocking
    void shutdown();

    // use this to expose a Config to remote control
    bool registerConfig(ConfigUtil::ConfigInterface* config);

    bool isRunning() const { return _running; }

    void takeInput(uint64_t session_id, Blobs& blobs);
    void giveOutput(uint64_t session_id, Blobs& blobs);

protected:
    uint64_t getMsecToNextFrame() const;
    void poll();

protected:
    ThreadPool _threads;

private:
    std::unique_ptr<Service> _service;
    uint64_t _framePeriod { DEFAULT_SERVER_FRAME_PERIOD }; // msec
    uint64_t _frameExpiry { 0 }; // msec
    bool _running {false};
    bool _stopped {false};
};


} // namespace mondo
