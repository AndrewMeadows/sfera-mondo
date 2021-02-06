//
// ConfigUtil.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
# pragma once

#include <filesystem>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

namespace ConfigUtil {

    class ConfigInterface {
    public:
        ConfigInterface() {}

        // name can be set non-empty only once
        void setName(const std::string& name) { if (_name.empty()) _name = name; }
        const std::string& getName() const { return _name; }

        // Get the config as a json
        virtual nlohmann::json getJson() const = 0;

        // get the config as a json string.
        // If 'indent' is nonnegative then array elements and object
        // members will be pretty: printed with that indent level.
        // An indent level of 0 will only insert newlines.
        // -1 (the default) selects the most compact representation.
        std::string getJsonString(int indent = -1) const;

        void updateJsonString(const std::string& json_str);
        virtual void updateJson(const nlohmann::json& json_obj) = 0;

        // set a filename for loading/saving to file
        void setFilename(const std::string& filename);
        std::string getFilename() const;

        // read/write return 'true' on success
        bool readFileIfChanged();
        bool writeFile();

        uint32_t getVersion() const { return _version; }

    protected:
        // call this in derived class after any internal parameter changes
        // to allow Config watchers to easily notice version mismatch
        void bumpVersion() { ++_version; }

        std::string _name;
        std::filesystem::path _filePath;
        std::filesystem::file_time_type _lastFileWrite;
        mutable std::mutex _mutex;
        uint32_t _version { 0 };
    };
} // namespace ConfigUtil

