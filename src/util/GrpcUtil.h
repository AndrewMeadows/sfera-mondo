//
// GrpcUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

namespace GrpcUtil {

// asynchronos call
//
// Using asynchronous Calls is recommended.
// It allows the client thread to put RPC requests on the wire
// and not block while waiting for response.
//
class Call {
public:
    virtual ~Call() {}

    virtual void start(grpc::CompletionQueue* queue, void* stub) = 0;
    virtual void processReply(bool reply_is_ok) = 0;
    virtual bool keepAlive() const { return false; }
    virtual void destroy() { delete this; }
    const grpc::Status& getRpcStatus() const { return _rpcStatus; }
    void cancel() { _context.TryCancel(); }

protected:
    // ClientContext for this call can be used to convey extra information
    // to the client/server and/or tweak certain RPC behaviors.
    // The context is a per-call object:
    // (a) it should NOT be recycled for other calls and
    // (b) it must remain alive and valid for the lifetime of this call
    grpc::ClientContext _context;
    grpc::Status _rpcStatus;
};

// asynchronous Client
//
// Using an asynchronous Client is recommended.
// It allows the client thread to put RPC requests on the wire
// and not block while waiting for response.
//
class Client {
public:
    Client(const std::string& uri);
    virtual ~Client();

    std::string getUri() const { return _uri; }

    virtual void start();
    void stop();
    bool isRunning() const { return _running; }
    bool isStopped() const { return _stopped; }

    // assumes ownership of Call
    void addCall(Call* call);

protected:
    void setStub(void* stub);

protected:
    std::unique_ptr<grpc::CompletionQueue> _queue;
    std::shared_ptr<grpc::Channel> _channel;
    std::string _uri;
    void* _stub { nullptr };
    bool _running { false };
    bool _stopped { true };
};


// asynchronous handler
//
// Using an asynchronous Server/Handler is for FANCY PURPOSES ONLY!
// The normal gRPC server architecture is already multi-threaded and can easily
// handle many requests per second (as long as the request handlers don't block
// for very long).
//
// The only reason I can think of for using custom asynchronous Server/Handler
// would be when your server runs into the aforementioned "block for a long
// time to compute the response" problem (for example: the when the response
// waits for a message from an entirely different thread/process/server, like
// a remote db query).  In this case you could shift all of that waiting onto a
// single poll-thread and then call back to custom ansychronous Handler scope
// when responses arrive.  gRPC will launch as many threads as it needs to
// handle incomming calls, could quickly dispatch them onto the slow poll, and
// could be ready for the next.
//
class Handler {
public:
    enum Status { CREATE, PROCESS, FINISH };

    virtual ~Handler() { }

    void proceed() {
        // proceed moves through a small state machine
        if (_status == CREATE) {
            // ask service to start processing requests with this-pointer as tag
            stageService();
            _status = PROCESS;
        } else if (_status == PROCESS) {
            // create a new handler for next request
            respawn();

            // do our dirty work with incomming data and prepare reply
            processRequest();

            // tell responder to send reply with this-pointer as tag
            finish();

            _status = FINISH;
        } else if (_status == FINISH) {
            // reply has been sent and service stops holding this-pointer
            destroy();
        }
    }

protected:
    virtual void stageService() = 0;
    virtual void respawn() = 0;
    virtual void processRequest() = 0;
    virtual void finish() = 0;

    void destroy() { delete this; } // yes: naked delete

protected:
    // Context for this RPC handler can be used to convey extra information
    // to the server/client and/or tweak certain RPC behaviors.
    // The context is a per-handler object:
    // (a) it should NOT be recycled for other handlers and
    // (b) it must remain alive and valid for the lifetime of this event
    grpc::ServerContext _context;

private:
    Status _status {CREATE};
};

// asynchronous Server
//
// Using an asynchronous Server is not recommended.
//
class Server {
public:
    Server() {}
    virtual ~Server();

    void buildService(uint32_t port);

    void start();
    void stop();

    int32_t getPort() const { return _port; }

    bool isRunning() const { return _running; }
    bool isStopped() const { return _stopped; }

protected:
    virtual void registerService(grpc::ServerBuilder& builder) = 0;

    // Note: registerService should look like this:
    //void registerService(grpc::ServerBuilder& builder) override {
    //    // Register services through which we'll communicate with clients.
    //    builder.RegisterService(&_service);
    //}

    virtual void spawnHandlers() = 0;

protected:
    std::unique_ptr<grpc::ServerCompletionQueue> _queue;

    // NOTE: derived class must instantiate _service like so:
    //foo::FubarService::AsyncService _service;

private:
    std::unique_ptr<grpc::Server> _grpcServer;
    int32_t _port { 0 };
    bool _running { false };
    bool _stopped { false };
};

} // namespace GrpcUtil


// suppose you hade a proto file like so:

/*
package foo;

message BarRequest {
  int32 baz = 1;
  string biz = 2;
}

message BarReply {
  int32 bazz = 1;
  string bizz = 2;
}

service FubarService {
  rpc Bar (BarRequest) returns (BarReply) {}
}
*/

// The GrpcUtil::Client implementation might look something like this:

/*
class FubarClient : public GrpcUtil::Client {
public:
    explicit FubarClient(const std::string& serverIpPort) :
            GrpcUtil::Client(serverIpPort),
            _stub(foo::FubarService::NewStub(_channel))
    {
        // as per the pattern we supply the base class
        // with a raw (void) pointer to our actual stub
        setStub(_stub.get());
    }

protected:
    // The Stub is the client's view of the server's exposed services
    std::unique_ptr<foo::FubarService::Stub> _stub;
};
*/

// The Caller would look like this:

/*
class BarCaller : public GrpcUtil::Caller {
public:
    BarCaller() {
        // set _request fields here
        // or from external context
        // but before calling start
    }

    void start(grpc::CompletionQueue* queue, void* stub) override {
        // cast the stub to the known type, as supplied by the Client
        foo::FubarService::Stub* service_stub = static_cast<foo::FubarService::Stub*>(stub);

        // stub->PrepareAsyncBar() creates an ResponseReader instance for RPC Bar
        // and returns its pointer, but does not actually start the RPC.
        // Because we are using the asynchronous API, we need to hold on
        // to the pointer until the call is complete.
        _listener = service_stub->PrepareAsyncBar(&_context, _request, queue);

        // NOTE: after PrepareAsyncBar() _context.peer() will report the recipient's
        // peer uri (e.g. "ipAddress:port")

        // StartCall initiates the RPC call
        _listener->StartCall();

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with this pointer.
        void* tag = this;
        _listener->Finish(&_reply, &_rpcStatus, tag);
    }

    void processReply(bool reply_is_ok) override {
        // check _rpcStatus to check for real reply
        bool status_ok = _rpcStatus.ok();
        EXPECT_TRUE(status_ok);
        if (status_ok) {
            // handle _reply here...
        }
    }

private:
    foo::BarRequest _request;
    foo::BarReply _reply;
    std::unique_ptr<grpc::ClientAsyncResponseReader<foo::BarReply> > _listener;
};
*/

// The Handler class might look like:
/*
class BarHandler : public GrpcUtil::Handler {
public:
    explicit BarHandler(
        foo::FubarService::AsyncService* service,
        grpc::ServerCompletionQueue* queue)
            :   GrpcUtil::Handler(),
                _responder(&_context),
                _service(service),
                _queue(queue)
    {
        GPR_ASSERT(_service != nullptr);
        GPR_ASSERT(_queue != nullptr);
        // the ctor() will init members and immediately call proceed()
        // which will feed this-pointer to service as "tag"
        proceed();
    }

protected:
    void processRequest() override {
        // get data from _request...
        // put data into _reply...
    }

    void stageService() override {
        // Ask service to start processing Bar requests.
        // NOTE: we "tag" the request with this-pointer and the service holds onto it.
        // The queue will receive two "events" with the tag:
        //   RequestReceived
        //   ReplySent
        // and will then drop the tag.
        void* tag = this;
        _service->RequestBar(&_context, &_request, &_responder, _queue, _queue, tag);
    }

     void respawn() override {
         // create new instance, which will feed its this-pointer to service
         new BarHandler(_service, _queue); // yes: naked new
     }

    void finish() override {
        // tell responder to send reply and tag with this-pointer
        void* tag = this;
        _responder.Finish(_reply, grpc::Status::OK, tag);
    }

private:
    grpc::ServerAsyncResponseWriter<foo::BarReply> _responder;
    foo::BarRequest _request;
    foo::BarReply _reply;
    foo::FubarService::AsyncService* _service;
    grpc::ServerCompletionQueue* _queue;
};
*/

// The Server implementation might look something like this:

/*
class Fubar_server : public GrpcUtil::Server {
public:
    Fubar_server(int32_t port) {
        buildService(port);
    }

    void spawnHandlers() override {
        // For each RPC call we handle:
        // Instantiate an Rpc_handler to wait for the next call.
        // The handler instance will add its this-pointer to the service as the "tag"
        // to be used in the next relevant event (e.g. incomming request of the right type).
        //
        // When the request arrives the Handler will instantiate its own replacement
        // before building a reply, and after sending reply the hander is expected to
        // call destroy() (which will typically cause it to delete itself).

        new Bar_handler(&_service, _queue.get()); // yes: naked new
    }

protected:
    void registerService(grpc::ServerBuilder& builder) override {
        // Register services through which we'll communicate with clients.
        builder.RegisterService(&_service);
    }

private:
    foo::FubarService::AsyncService _service;
};
*/

