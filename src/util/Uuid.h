//
// Uuid.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <string>

class Uuid {
public:
    static Uuid newUuid();

    Uuid() { }
    ~Uuid() { }

    void generate();

    bool isNull() const { return _data[0] == 0 && _data[1] == 0; }

    bool operator==(const Uuid& other) const;
    bool operator>(const Uuid& other) const;
    bool operator<(const Uuid& other) const;

    void setXor(const Uuid& other);

    std::string toString4122() const;
    bool fromString4122(const std::string& str);

    std::string toStringEzHex() const; // for debug purposes

    // writes 16 bytes starting at 'buffer'
    void toRawData(uint8_t* buffer) const;

    // reads 16 bytes starting at 'source'
    void fromRawData(const uint8_t* buffer);

private:
    uint64_t _data[2] { 0, 0 };
};
