//
// test_ConfigUtil.cpp
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or
//  http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gtest/gtest.h>

#include <util/ConfigUtil.h>
#include <util/RandomUtil.h>

using json = nlohmann::json;
using Datum = glm::quat;

// helper
Datum datum_from_json(const json& obj) {
    Datum q(1.0f, 0.0f, 0.0f, 0.0f);
    if (obj.is_array() && obj.size() == 4) {
        q.w = obj[0];
        q.x = obj[1];
        q.y = obj[2];
        q.z = obj[3];
    }
    return q;
}

// helper
json datum_to_json(const Datum& q) {
    json obj = json::array();
    obj.push_back(q.w);
    obj.push_back(q.x);
    obj.push_back(q.y);
    obj.push_back(q.z);
    return obj;
}

// helper
Datum random_datum() {
    constexpr float ANGLE = 1.57f; // ~PI/2
    glm::vec3 axis = RandomUtil::unitSphereSurface();
    return glm::angleAxis(ANGLE, axis);
}

// helper
bool approximately_equal(const Datum& A, const Datum& B) {
    constexpr float DOT_PRODUCT_SLOP = 8.0e-7; // ~0.1 degree of error
    float dot_product = std::abs(glm::dot(A, B));
    return std::abs(1.0f - dot_product) < DOT_PRODUCT_SLOP;
}

class TestConfig : public ConfigUtil::ConfigInterface {
public:
    TestConfig() { }

    ~TestConfig() { }

    // required overrides
    nlohmann::json getJson() const override {
        json obj;
        obj["word"] = _word;
        obj["number"] = _number;
        json data = json::array();
        for (size_t i = 0; i < _data.size(); ++i) {
            data.push_back(datum_to_json(_data[i]));
        }
        obj["data"] = data;
        return obj;
    }

    void updateJson(const nlohmann::json& obj) override {
        bool something_changed = false;
        if (obj.contains("word") && obj["word"].is_string()) {
            _word = obj["word"];
            something_changed = true;
        }
        if (obj.contains("number") && obj["number"].is_number()) {
            _number = obj["number"];
            something_changed = true;
        }
        if (obj.contains("data") && obj["data"].is_array()) {
            _data.clear();
            json data = obj["data"];
            _data.reserve(data.size());
            for (json::iterator i = data.begin(); i != data.end(); ++i) {
                _data.push_back(datum_from_json(*i));
            }
            something_changed = true;
        }
        if (something_changed) {
            bumpVersion();
        }
    }

    void setWord(const std::string& word) {
        if (word != _word) {
            _word = word;
            bumpVersion();
        }
    }

    void setNumber(int32_t number) {
        if (number != _number) {
            _number = number;
            bumpVersion();
        }
    }

    void setData(const std::vector<Datum>& data) {
        bool something_changed = false;
        size_t num_data = data.size();
        if (num_data != _data.size()) {
            something_changed = true;
            _data.clear();
        }
        _data.reserve(num_data);

        size_t i = 0;
        while (i < num_data) {
            if (i < _data.size()) {
                if (_data[i] != data[i]) {
                    _data[i] = data[i];
                    something_changed = true;
                }
            } else {
                _data.push_back(data[i]);
            }
            ++i;
        }
        if (something_changed) {
            bumpVersion();
        }
    }

    bool operator==(const TestConfig& other) {
        if (_word != other._word
            || _number != other._number
            || _data.size() != other._data.size())
        {
            return false;
        }
        for (size_t i = 0; i < _data.size(); ++i) {
            if (!approximately_equal(_data[i], other._data[i])) {
                return false;
            }
        }
        return true;
    }

private:
    std::string _word;
    int32_t _number { 0 };
    std::vector<Datum> _data;
};

TEST(ConfigUtil_test, read_write) {
    // make some data to add to the config
    size_t num_data = 16;
    std::vector<Datum> data;
    data.reserve(num_data);
    for (size_t i = 0; i < num_data; ++i) {
        data.push_back(random_datum());
    }

    // make a config and fill it
    TestConfig config_A;
    config_A.setWord("foo");
    config_A.setNumber(1);
    config_A.setData(data);

    // make another config and leave it empty
    TestConfig config_B;
    EXPECT_FALSE(config_A == config_B);
    EXPECT_FALSE(config_B == config_A);

    // copy first to second via json
    json obj = config_A.getJson();
    config_B.updateJson(obj);
    EXPECT_TRUE(config_A == config_B);
    EXPECT_TRUE(config_B == config_A);

    // modify the first
    config_A.setWord("bar");
    EXPECT_FALSE(config_A == config_B);

    // copy first to second via file
    std::string filename = fmt::format("/tmp/test_ConfigUtil-{:0<8x}.json", RandomUtil::uint32());
    config_A.setFilename(filename);
    bool success = config_A.writeFile();
    EXPECT_TRUE(success);

    TestConfig config_C;
    config_C.setFilename(filename);
    success = config_C.readFileIfChanged();
    EXPECT_TRUE(success);
    EXPECT_TRUE(config_A == config_C);

    success = std::filesystem::remove(filename);
    EXPECT_TRUE(success);
}

int main(int32_t argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
