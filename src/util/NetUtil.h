//
// NetUtil.h
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <string>

namespace NetUtil {

const std::string IPV6_ANY = "[::]";
const std::string IPV6_LOOPBACK = "[::1]";

// returns true if port can be opened
bool port_is_available(int32_t port);

// returns a port that can be opened, else 0
int32_t find_available_port();

// old_uri = "ip_address:xx"
// new_port = ":yy"
// returns new_uri = "ip_address:yy"
std::string compute_new_uri(
        const std::string& old_uri,
        const std::string& new_port);

bool port_is_valid(int32_t port);

bool uri_is_valid(const std::string& uri);

bool ip_port_from_uri(
        const std::string& uri,
        std::string& ip, int32_t& port);

#ifdef WIN32
// On windows, Winsock must be initialized once per process
// Allocate a new WinsockInitializer at the begining of the program before any other calls from NetUtil
// Make sure to destroy last.
//
class WinsockInitializer {
public:
    WinsockInitializer();
    ~WinsockInitializer();
    int error() { return err_; }

private:
    int err_;
};

#endif

} // NetUtil namespace
