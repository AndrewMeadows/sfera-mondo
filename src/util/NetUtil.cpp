//
// NetUtil.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NetUtil.h"

#include <fmt/format.h>

#if __has_include("unistd.h")
#  include <unistd.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

#ifdef WIN32
#  include <winsock2.h>
#  include <WS2tcpip.h>
#  include <io.h>

NetUtil::WinsockInitializer::WinsockInitializer() {
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 0);
    err_ = WSAStartup(wVersionRequested, &wsaData);
}

NetUtil::WinsockInitializer::~WinsockInitializer() {
    if (!err_)
        WSACleanup();
}
#endif

bool NetUtil::port_is_available(int32_t port) {
    int32_t handle;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addr_len = sizeof(addr);

    handle = socket(PF_INET, SOCK_STREAM, 0);

    // try to bind
    int32_t code = bind(handle, (struct sockaddr *) &addr, addr_len);
    if (0 == code) {
        // bind() was successful
#ifdef WIN32
        closesocket(handle);
#else
        close(handle);
#endif
        return true;
    }
    return false;
}

int32_t NetUtil::find_available_port() {
    int32_t port = 0; // invalid port
    int32_t handle;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addr_len = sizeof(addr);

    handle = socket(PF_INET, SOCK_STREAM, 0);

    // since we supply an invalid port (e.g. 0)
    // bind() will find an unused port for us
    int32_t code = bind(handle, (struct sockaddr *) &addr, addr_len);
    if (0 == code) {
        // bind() was successful and found an available port
        code = getsockname(handle, (struct sockaddr *) &addr, &addr_len);
        if (code == 0) {
            // getsockname() was successful, so we pull the port number
            // from addr, but we must swap "NetUtil-order" for "host order"
            port = (int32_t)(ntohs(addr.sin_port));
        }
        // now that we've verified we can bind to this port
        // close it and return the value
        // We expect the recipient to be able to re-bind to it.
#ifdef WIN32
        closesocket(handle);
#else
        close(handle);
#endif
    }
    return port > 0 ? port : 0;
}

// old_uri = "ip_address:xx"
// new_port = ":yy"
// returns new_uri = "ip_address:yy"
std::string NetUtil::compute_new_uri(
        const std::string& old_uri,
        const std::string& new_port)
{
    std::size_t pos = old_uri.find_last_of(":");
    if (pos != std::string::npos) {
        return fmt::format("{}{}", old_uri.substr(0,pos), new_port);
    }
    // else assume old_uri is valid ip_address sans colons
    // and is just missing ":port"
    return fmt::format("{}{}", old_uri, new_port);
}

bool NetUtil::port_is_valid(int32_t port) {
    constexpr int32_t MIN_EPHEMERAL_PORT = 1024;
    constexpr int32_t MAX_EPHEMERAL_PORT = 65535;
    return port >= MIN_EPHEMERAL_PORT && port <= MAX_EPHEMERAL_PORT;
}

bool NetUtil::uri_is_valid(const std::string& uri) {
    // here are some examples of good uris:
    // uri = "ipv4:192.168.1.79:50051"
    // uri = "ipv6:[::]:50051"
    std::size_t pos = uri.find_last_of(":");
    if (pos == std::string::npos) {
        //fmt::print("bad uri='{}' for no colon\n", uri);
        return false;
    }
    std::size_t uri_len = uri.length();
    if (pos == uri_len - 1) {
        //fmt::print("bad uri='{}' for no port\n", uri);
        return false;
    }
    std::string ip = uri.substr(0, pos);
    if (ip.empty()) {
        //fmt::print("bad uri='{}' for no ip_address\n", uri);
        return false;
    }
    int32_t port = std::stoi(uri.substr(pos + 1, uri_len));
    if (!NetUtil::port_is_valid(port)) {
        //fmt::print("bad uri='{}' for out-of-range port={}\n", uri, port);
        return false;
    }
    return true;
}

bool NetUtil::ip_port_from_uri(
        const std::string& uri,
        std::string& ip, int32_t& port)
{
    std::size_t pos = uri.find_last_of(":");
    if (pos == std::string::npos) {
        //fmt::print("bad uri='{}' for no colon\n", uri);
        return false;
    }
    std::size_t uri_len = uri.length();
    if (pos == uri_len - 1) {
        //fmt::print("bad uri='{}' for no port\n", uri);
        return false;
    }
    std::string a = uri.substr(0, pos);
    if (a.empty()) {
        //fmt::print("bad uri='{}' for no ip_address\n", uri);
        return false;
    }
    int32_t p = std::stoi(uri.substr(pos + 1, uri_len));
    if (!NetUtil::port_is_valid(p)) {
        //fmt::print("bad uri='{}' for out-of-range port={}\n", uri, p);
        return false;
    }
    ip = a;
    port = p;
    return true;
}
