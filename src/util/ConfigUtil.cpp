//
// ConfigUtil.cpp
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ConfigUtil.h"

#include <fstream>

#include "LogUtil.h"

using namespace ConfigUtil;
using json = nlohmann::json;

// set a filename for loading/saving to file
void ConfigInterface::setFilename(const std::string& filename) {
    _filePath.assign(filename);
    _lastFileWrite = std::filesystem::file_time_type();
}

std::string ConfigInterface::getFilename() const {
    return _filePath.string();
}

std::string ConfigInterface::getJsonString(int indent) const {
    return getJson().dump(indent);
}

void ConfigInterface::updateJsonString(const std::string& json_str) {
    json json_obj = json::parse(json_str);
    update_json(json_obj);
}

bool ConfigInterface::readFile() {
    if (_filePath.empty()) {
        return false;
    }

    try {
        auto last_write = std::filesystem::last_write_time(_filePath);
        if (last_write != _lastFileWrite) {
            // file exists and has changed
            std::ifstream input_stream;
            std::unique_lock<decltype(_mutex)> lock(_mutex);
            input_stream.open(_filePath.string());
            if (input_stream.is_open()) {
                LOG1("read from config_file='{}'\n", _filePath.string());
                json json_obj;
                input_stream >> json_obj;
                input_stream.close();
                update_json(json_obj);
                _lastFileWrite = last_write;
            } else {
                LOG1("unable to read from config_file='{}'\n", _filePath.string());
                return false;
            }
        }
    } catch (const std::exception& e) {
        // file doesn't exist: normal flow, not an error
        //LOG1("error: config_file='{}' err='{}'\n", _filePath.string(), e.what());
        return false;
    }
    return true;
}

bool ConfigInterface::writeFile() {
    if (_filePath.empty()) {
        return false;
    }
    std::ofstream output_stream;
    std::unique_lock<decltype(_mutex)> lock(_mutex);
    output_stream.open(_filePath.string());
    if (output_stream.is_open()) {
        std::string json_str = getJsonString(2);
        json_str.append("\n");
        output_stream << json_str;
        output_stream.close();
        LOG1("write config_file='{}'\n", _filePath.string());
        try {
            _lastFileWrite = std::filesystem::last_write_time(_filePath);
        } catch (const std::exception& e) {
            LOG1("error: config_file='{}' err='{}'\n", _filePath.string(), e.what());
        }
        return true;
    } else {
        LOG1("unable to write to config_file='{}'\n", _filePath.string());
    }
    return false;
}

