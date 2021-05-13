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

#include "Server.h"

using namespace mondo;

Server::Server(const ConfigUtil::ConfigInterface* config) : _threads(2) {
}

// use this to expose a Config to remote control
bool Server::registerConfig(ConfigUtil::ConfigInterface* config) {
    return false;
}

void Server::shutdown() {
}

void Server::takeInput(uint64_t session_id, Blobs& blobs) {
}

void Server::giveOutput(uint64_t session_id, Blobs& blobs) {
}


void Server::runServiceThread() {
}

void Server::runPollingThread() {
}

void Server::runShutdownThread() {
}

