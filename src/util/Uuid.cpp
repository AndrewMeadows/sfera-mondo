//
// Uuid.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Uuid.h"

#include <random>

#include <fmt/format.h>
#include <string.h>

std::random_device rd;  // used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

void Uuid::generate() {
    _data[0] = dis(gen); // low half
    _data[1] = dis(gen); // high half
}

bool Uuid::operator==(const Uuid& other) const {
    return _data[0] == other._data[0] && _data[1] == other._data[1];
}

bool Uuid::operator>(const Uuid& other) const {
    // assume LITTLE_ENDIAN
    return _data[1] > other._data[1]
        || (_data[1] == other._data[1] && _data[0] > other._data[0]);
}

bool Uuid::operator<(const Uuid& other) const {
    // assume LITTLE_ENDIAN
    return _data[1] < other._data[1]
        || (_data[1] == other._data[1] && _data[0] < other._data[0]);
}

void Uuid::setXor(const Uuid& other) {
    _data[0] ^= other._data[0];
    _data[1] ^= other._data[1];
}

std::string Uuid::toString4122() const {
    // The string will have the following form (as per RFC-4122)
    //
    // "01234567-89ab-cdef-dead-beefdeadbeef"
    //
    // for LITTLE_ENDIAN _data bytes are laid out in memory like this:
    //
    //           _data[0]                _data[1]
    //  ef cd ab 89 67 54 23 01     ef be ad da fe be ad ed
    //
    // The string memory is laid out like so:
    //
    //  0                        low ----> high                               35
    // "0 1 2 3 4 5 6 7 - 8 9 a b - c d e f - d e a d - b e e f d e a d b e e f"

    uint8_t* bytes = (uint8_t*)_data;
    uint8_t low_mask = 0x0f;
    uint8_t high_mask = 0xf0;
    std::string s = "01234567-0123-0123-0123-0123456789ab";
    int32_t i = 0; // bytes_index
    int32_t j = 35; // str_index
    const char* hex_digits = "0123456789abcdef";
    for (i = 0; i < 6; ++i) {
        s[j--] = hex_digits[(bytes[i] & high_mask) >> 4];
        s[j--] = hex_digits[bytes[i] & low_mask];
    }
    s[j--] = '-';
    for (i = 6; i < 8; ++i) {
        s[j--] = hex_digits[(bytes[i] & high_mask) >> 4];
        s[j--] = hex_digits[bytes[i] & low_mask];
    }
    s[j--] = '-';
    for (i = 8; i < 10; ++i) {
        s[j--] = hex_digits[(bytes[i] & high_mask) >> 4];
        s[j--] = hex_digits[bytes[i] & low_mask];
    }
    s[j--] = '-';
    for (i = 10; i < 12; ++i) {
        s[j--] = hex_digits[(bytes[i] & high_mask) >> 4];
        s[j--] = hex_digits[bytes[i] & low_mask];
    }
    s[j--] = '-';
    for (i = 12; i < 16; ++i) {
        s[j--] = hex_digits[(bytes[i] & high_mask) >> 4];
        s[j--] = hex_digits[bytes[i] & low_mask];
    }
    return s;
}

// helper
bool char_to_low_nibble(uint8_t c, uint8_t* byte) {
    uint8_t high_mask = 0xf0;
    if (c >= '0' && c <= '9') {
        *byte = ((*byte) & high_mask) + (c - '0');
        return true;
    } else if (c >= 'a' && c <= 'f') {
        *byte = ((*byte) & high_mask) + (c - 'a' + 10);
        return true;
    } else if (c >= 'A' && c <= 'F') {
        *byte = ((*byte) & high_mask) + (c - 'A' + 10);
        return true;
    }
    return false;
}

// helper
bool char_to_high_nibble(uint8_t c, uint8_t* byte) {
    uint8_t low_mask = 0x0f;
    if (c >= '0' && c <= '9') {
        *byte = ((*byte) & low_mask) + ((c - '0') << 4);
        return true;
    } else if (c >= 'a' && c <= 'f') {
        *byte = ((*byte) & low_mask) + ((c - 'a' + 10) << 4);
        return true;
    } else if (c >= 'A' && c <= 'F') {
        *byte = ((*byte) & low_mask) + ((c - 'A' + 10) << 4);
        return true;
    }
    return false;
}

bool Uuid::fromString4122(const std::string& str) {
    if (str.size() != 36) {
        return false;
    }

    // validate as we go, and only copy to _data if everthing looks good
    uint64_t data[2];
    uint8_t* bytes = (uint8_t*)data;
    int32_t i = 35; // string_index
    int32_t j = 0; // data_index
    for (int32_t k = 0; k < 6; ++k) {
        if (!char_to_high_nibble(str[i--], bytes + j)
            || !char_to_low_nibble(str[i--], bytes + j)) {
            return false;
        }
        j++;
    }
    if (str[i--] != '-') {
        return false;
    }
    for (int32_t k = 0; k < 2; ++k) {
        if (!char_to_high_nibble(str[i--], bytes + j)
            || !char_to_low_nibble(str[i--], bytes + j)) {
            return false;
        }
        j++;
    }
    if (str[i--] != '-') {
        return false;
    }
    for (int32_t k = 0; k < 2; ++k) {
        if (!char_to_high_nibble(str[i--], bytes + j)
            || !char_to_low_nibble(str[i--], bytes + j)) {
            return false;
        }
        j++;
    }
    if (str[i--] != '-') {
        return false;
    }
    for (int32_t k = 0; k < 2; ++k) {
        if (!char_to_high_nibble(str[i--], bytes + j)
            || !char_to_low_nibble(str[i--], bytes + j)) {
            return false;
        }
        j++;
    }
    if (str[i--] != '-') {
        return false;
    }
    for (int32_t k = 0; k < 4; ++k) {
        if (!char_to_high_nibble(str[i--], bytes + j)
            || !char_to_low_nibble(str[i--], bytes + j)) {
            return false;
        }
        j++;
    }

    // finally copy into _data
    _data[0] = data[0];
    _data[1] = data[1];
    return true;
}

std::string Uuid::toStringEzHex() const {
    // for debug purposes
    // print 64-bit chunks in easy-to-read hex format:
    //
    // "01 23 45 67 89 ab cd ef : de ad be ef de ad be ef"
    std::string str;
    uint8_t* bytes = (uint8_t*)_data;
    for (int32_t i = 0; i < 8; ++i) {
        str.append(fmt::format("{:0>2x} ", bytes[i]));
    }
    str.append(":");
    for (int32_t i = 8; i < 16; ++i) {
        str.append(fmt::format(" {:0>2x}", bytes[i]));
    }
    return str;
}

// writes 16 bytes starting at 'buffer'
void Uuid::toRawData(uint8_t* buffer) const {
    memcpy(buffer, _data, 16);
}

// reads 16 bytes starting at 'source'
void Uuid::fromRawData(const uint8_t* buffer) {
    memcpy(_data, buffer, 16);
}
