//
// mondo/Service.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <memory>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <autogen/mondo.grpc.pb.h>

namespace mondo {

// SessionManager is interface for logins
class SessionManager {
public:
    SessionManager() {}
    uint64_t getOrAddSessionId(const std::string& u, const std::string& p) const {
        return 0;
    }

    bool isValid(uint64_t id) const {
        return false;
    }

    bool endSessionById(uint64_t id) {
        return true;
    }
};

using Data = google::protobuf::RepeatedPtrField<Blob>;

// DataExchange is arbiter for passing Data in (show) and out (borrow)
// across thread boundaries for various session ids.
class DataExchange {
public:
    DataExchange() {
    }

    bool registerId(uint64_t id) {
        return false;
    }

    bool deregisterId(uint64_t id) {
        return false;
    }

    void showData(uint64_t id, Data const* data) {
    }

    const Data* borrowData(uint64_t id) {
        return nullptr;
    }

    void endBorrow(const Data* data) {
    }

private:
    //std::unordered_map<uint64_t, DataChannel> _dataChannels;
};

// Service implements the missing DataService::Service methods.
class Service : public DataService::Service {
public:

    Service(int32_t port=0);

    void setSessionManager(SessionManager* sessions) {
        _sessionManager = sessions;
    }

    void setDataExchange(DataExchange* exchange) {
        _dataExchange = exchange;
    }

    // start() will block (doing threaded work) until stopped
    // (e.g. call start() on devoted thread)
    void start();

    // stop() will block until threaded work is done
    void stop();

    int32_t getPort() const { return _grpcServicePort; }
    bool isRunning() const { return _running; }

    // rpc StartSession (LoginRequest) returns (Input) {}
    ::grpc::Status StartSession(
            ::grpc::ServerContext* context,
            const LoginRequest* request,
            Input* response) override final;

    // rpc EndSession (Input) returns (Output) {}
    ::grpc::Status EndSession(
            ::grpc::ServerContext* context,
            const Input* request,
            Output* response) override final;

    // rpc PollInOut (Input) returns (Output) {}
    ::grpc::Status PollInOut(
            ::grpc::ServerContext* context,
            const Input* request,
            Output* response) override final;

    /*
    // rpc StreamIn (stream Input) returns (Output) {}
    ::grpc::Status StreamIn(
            ::grpc::ServerContext* context,
            const Input* request,
            Output* response) override final;

    // rpc StreamOut (Input) returns (stream Output) {}
    ::grpc::Status StreamOut(
            ::grpc::ServerContext* context,
            const Input* request,
            Output* response) override final;

    // rpc StreamInOut (stream Input) returns (stream Output) {}
    ::grpc::Status StreamInOut(
            ::grpc::ServerContext* context,
            const Input* request,
            Output* response) override final;
            */

private:
    std::unique_ptr<grpc::Server> _grpcServer;
    SessionManager* _sessionManager { nullptr };
    DataExchange* _dataExchange { nullptr };
    int32_t _grpcServicePort { 0 };
    bool _running { false };
    bool _stopped { true };
};

} // namespace mondo
