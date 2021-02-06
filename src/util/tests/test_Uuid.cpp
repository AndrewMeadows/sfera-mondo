//
// test_Uuid.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <string>

#include <gtest/gtest.h>

#include <util/Uuid.h>
#include <util/RandomUtil.h>

TEST(Uuid_test, isNull) {
    Uuid A;
    EXPECT_TRUE(A.isNull());
    A.generate();
    EXPECT_FALSE(A.isNull());
}

TEST(Uuid_test, generate) {
    Uuid A = Uuid::newUuid();
    Uuid B = Uuid::newUuid();
    EXPECT_FALSE(A == B);
}

TEST(Uuid_test, serialize_to_buffer) {
    Uuid A = Uuid::newUuid();
    uint8_t buffer[16];
    A.toRawData(buffer);

    Uuid B;
    B.fromRawData(buffer);
    EXPECT_TRUE(A == B);
}

TEST(Uuid_test, string_4122) {
    std::string id = "deadbeef-feed-fade-abba-facebeedcede";
    Uuid A;
    bool success = A.fromString4122(id);
    EXPECT_TRUE(success);
    EXPECT_FALSE(A.isNull());

    std::string A_as_string = A.toString4122();
    EXPECT_TRUE(id == A_as_string);

    Uuid B;
    B.fromString4122(A_as_string);
    EXPECT_TRUE(A == B);
}

TEST(Uuid_test, comparison) {
    std::string bit0 = "00000000-0000-0000-0000-000000000001";
    std::string bit63 = "00000000-0000-0000-1000-000000000000";
    std::string bit64 = "00000000-0000-0001-0000-000000000000";
    std::string bit127 = "10000000-0000-0000-0000-000000000000";

    Uuid A, B, C, D, E;
    B.fromString4122(bit0);
    C.fromString4122(bit63);
    D.fromString4122(bit64);
    E.fromString4122(bit127);

    EXPECT_TRUE(A < B);
    EXPECT_TRUE(A < C);
    EXPECT_TRUE(A < D);
    EXPECT_TRUE(A < E);

    EXPECT_TRUE(B > A);
    EXPECT_TRUE(B < C);
    EXPECT_TRUE(B < D);
    EXPECT_TRUE(B < E);

    EXPECT_TRUE(C > A);
    EXPECT_TRUE(C > B);
    EXPECT_TRUE(C < D);
    EXPECT_TRUE(C < E);

    EXPECT_TRUE(D > A);
    EXPECT_TRUE(D > B);
    EXPECT_TRUE(D > C);
    EXPECT_TRUE(D < E);

    EXPECT_TRUE(E > A);
    EXPECT_TRUE(E > B);
    EXPECT_TRUE(E > C);
    EXPECT_TRUE(E > D);
}

int main(int32_t argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
