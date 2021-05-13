//
// mondo/Server.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
# pragma once

#include <vector>

//#include <autogen/mondo_grpc_pb.h>
#include <util/ConfigUtil.h>
#include <util/ThreadPool.h>

#include "Service.h"

namespace mondo {

using Blobs = std::vector<Blob>;

// Server has a Service and relays Blobs.
class Server {
public:
    Server(const ConfigUtil::ConfigInterface* config);

    // use this to expose a Config to remote control
    bool registerConfig(ConfigUtil::ConfigInterface* config);

    bool isRunning() const { return _isRunning; }
    void stop() { _isRunning = false; }
    void shutdown();

    void takeInput(uint64_t session_id, Blobs& blobs);
    void giveOutput(uint64_t session_id, Blobs& blobs);

protected:
    void runServiceThread();
    void runPollingThread();
    void runShutdownThread();

protected:
    ThreadPool _threads;

private:
    //std::unique_ptr<Data_store> _data_store;
    //std::unique_ptr<Sora_service> _sora_service;
    //std::unique_ptr<Sora_engine> _sora_engine;
    Service _service;
    bool _isRunning {false};
    bool _isStopped {false};
};


} // namespace mondo
