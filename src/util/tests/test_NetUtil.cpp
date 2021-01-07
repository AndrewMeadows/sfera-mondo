//
// test_NetUtil.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <util/NetUtil.h>

TEST(NetUtil_test, compute_new_uri_well_formatted) {
    { // well formatted
        std::string uri = "127.0.0.1:1234";
        std::string port = ":5678";
        std::string new_uri = NetUtil::compute_new_uri(uri, port);
        EXPECT_EQ(new_uri, "127.0.0.1:5678");
    }
    { // no port digits
        std::string uri = "127.0.0.1:";
        std::string port = ":456";
        std::string new_uri = NetUtil::compute_new_uri(uri, port);
        EXPECT_EQ(new_uri, "127.0.0.1:456");
    }
    { // no :port
        std::string uri = "127.0.0.1";
        std::string port = ":456";
        std::string new_uri = NetUtil::compute_new_uri(uri, port);
        EXPECT_EQ(new_uri, "127.0.0.1:456");
    }
}

TEST(NetUtil_test, uri_is_valid) {
    EXPECT_FALSE(NetUtil::uri_is_valid("127.0.0.1")); // no colon
    EXPECT_FALSE(NetUtil::uri_is_valid("127.0.0.1:")); // no port digits
    EXPECT_TRUE(NetUtil::uri_is_valid("127.0.0.1:1234"));
    EXPECT_FALSE(NetUtil::uri_is_valid(":1234")); // no ip
    EXPECT_FALSE(NetUtil::uri_is_valid("127.0.0.1:1023")); // port too low
    EXPECT_TRUE(NetUtil::uri_is_valid("127.0.0.1:1024"));
    EXPECT_TRUE(NetUtil::uri_is_valid("127.0.0.1:65535"));
    EXPECT_FALSE(NetUtil::uri_is_valid("127.0.0.1:65536")); // port too high
}

TEST(NetUtil_test, ip_port_from_uri) {
    { // well formatted
        std::string ip = "";
        int32_t port = -1;
        EXPECT_TRUE(NetUtil::ip_port_from_uri("1.2.3.4:5678", ip, port));
        EXPECT_EQ("1.2.3.4", ip);
        EXPECT_EQ(5678, port);
    }
    { // no ip:
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri("5678", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
    { // no ip
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri(":5678", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
    { // port too low
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri("1.2.3.4:678", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
    { // port too high
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri("1.2.3.4:65536", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
    { // no :port
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri("1.2.3.4", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
    { // no port
        std::string ip = "";
        int32_t port = -1;
        EXPECT_FALSE(NetUtil::ip_port_from_uri("1.2.3.4:", ip, port));
        EXPECT_EQ("", ip);
        EXPECT_EQ(-1, port);
    }
}
