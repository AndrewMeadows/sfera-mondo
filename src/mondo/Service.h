//
// mondo/Service.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <memory>

#include <grpcpp/grpcpp.h>
#include <autogen/mondo.grpc.pb.h>

namespace mondo {

// Service implements the missing DataService::Service methods.
class Service : public DataService::Service {
public:

    Service(int32_t port=0);

    // call start() on devoted thread
    void start();

    // stop() will block until threaded work is done
    void stop();

    int32_t getPort() const { return _grpcServicePort; }
    bool isRunning() const { return _running; }

    // rpc StartSession (LoginRequest) returns (Input) {}
    grpc::Status StartSession(
            grpc::ServerContext* context,
            const mondo::LoginRequest* request,
            mondo::Input* reply) override final;

    // rpc PollInOut (Input) returns (Output) {}
    grpc::Status PollInOut(
            grpc::ServerContext* context,
            const mondo::Input* request,
            mondo::Ouput* reply) override final;

    /*
    // rpc StreamIn (stream Input) returns (Output) {}
    grpc::Status StreamIn(
            grpc::ServerContext* context,
            const mondo::Input* request,
            mondo::Ouput* reply) override final;

    // rpc StreamOut (Input) returns (stream Output) {}
    grpc::Status StreamOut(
            grpc::ServerContext* context,
            const mondo::Input* request,
            mondo::Ouput* reply) override final;

    // rpc StreamInOut (stream Input) returns (stream Output) {}
    grpc::Status StreamInOut(
            grpc::ServerContext* context,
            const mondo::Input* request,
            mondo::Ouput* reply) override final;
            */

private:
    std::unique_ptr<grpc::Server> _grpcServer;
    int32_t _grpcServicePort { 0 };
    bool _running { false };
    bool _stopped { true };
};

} // namespace mondo
