/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "simple_tftpd/tftp_config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>
#ifdef HAS_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif
#include <regex>

namespace simple_tftpd {

TftpConfig::TftpConfig() {
    setDefaults();
}

TftpConfig::~TftpConfig() = default;

void TftpConfig::setDefaults() {
    // Network settings
    listen_address_ = "0.0.0.0";
    listen_port_ = TFTP_DEFAULT_PORT;
    ipv6_enabled_ = true;
    
    // File system settings
    root_directory_ = "/var/tftp";
    allowed_directories_.clear();
    
    // Security settings
    read_enabled_ = true;
    write_enabled_ = false;
    max_file_size_ = 100 * 1024 * 1024; // 100MB
    overwrite_protection_ = true;
    
    // Performance settings
    block_size_ = 512;
    timeout_ = 5;
    window_size_ = 1;
    
    // Logging settings
    log_level_ = LogLevel::INFO;
    log_file_ = "";
    console_logging_ = true;
}

bool TftpConfig::loadFromFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // Determine format by extension
    std::string ext;
    auto dot = config_file.find_last_of('.');
    if (dot != std::string::npos) {
        ext = config_file.substr(dot + 1);
        for (auto& c : ext) c = static_cast<char>(::tolower(c));
    }
    
    if (ext == "json") {
        return loadFromJson(buffer.str());
    } else if (ext == "yaml" || ext == "yml") {
        return parseYaml(buffer.str());
    } else if (ext == "ini" || ext == "conf") {
        return parseIni(buffer.str());
    }
    
    // Fallback: try JSON, then YAML, then INI heuristically
    if (loadFromJson(buffer.str())) return true;
    if (parseYaml(buffer.str())) return true;
    if (parseIni(buffer.str())) return true;
    return false;
}

bool TftpConfig::loadFromJson(const std::string& json_config) {
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        std::istringstream json_stream(json_config);
        
        if (!Json::parseFromStream(builder, json_stream, &root, &errors)) {
            return false;
        }
        
        return parseJson(root);
    } catch (const std::exception& e) {
        return false;
    }
}
// Minimal YAML parsing using yaml-cpp if available; otherwise very basic fallback
bool TftpConfig::parseYaml(const std::string& yaml_config) {
#ifdef HAS_YAML_CPP
    try {
        YAML::Node root = YAML::Load(yaml_config);
        // Convert YAML::Node to Json::Value-like structure and reuse parseJson-style logic
        Json::Value jsonRoot;
        // network
        if (root["network"]) {
            auto net = root["network"];
            if (net["listen_address"]) jsonRoot["network"]["listen_address"] = net["listen_address"].as<std::string>();
            if (net["listen_port"]) jsonRoot["network"]["listen_port"] = net["listen_port"].as<unsigned int>();
            if (net["ipv6_enabled"]) jsonRoot["network"]["ipv6_enabled"] = net["ipv6_enabled"].as<bool>();
        }
        if (root["filesystem"]) {
            auto fs = root["filesystem"];
            if (fs["root_directory"]) jsonRoot["filesystem"]["root_directory"] = fs["root_directory"].as<std::string>();
            if (fs["allowed_directories"]) {
                for (auto it : fs["allowed_directories"]) {
                    jsonRoot["filesystem"]["allowed_directories"].append(it.as<std::string>());
                }
            }
        }
        if (root["security"]) {
            auto sec = root["security"];
            if (sec["read_enabled"]) jsonRoot["security"]["read_enabled"] = sec["read_enabled"].as<bool>();
            if (sec["write_enabled"]) jsonRoot["security"]["write_enabled"] = sec["write_enabled"].as<bool>();
            if (sec["max_file_size"]) jsonRoot["security"]["max_file_size"] = (Json::UInt64)sec["max_file_size"].as<unsigned long long>();
            if (sec["overwrite_protection"]) jsonRoot["security"]["overwrite_protection"] = sec["overwrite_protection"].as<bool>();
        }
        if (root["performance"]) {
            auto perf = root["performance"];
            if (perf["block_size"]) jsonRoot["performance"]["block_size"] = perf["block_size"].as<unsigned int>();
            if (perf["timeout"]) jsonRoot["performance"]["timeout"] = perf["timeout"].as<unsigned int>();
            if (perf["window_size"]) jsonRoot["performance"]["window_size"] = perf["window_size"].as<unsigned int>();
        }
        if (root["logging"]) {
            auto log = root["logging"];
            if (log["level"]) jsonRoot["logging"]["level"] = log["level"].as<std::string>();
            if (log["log_file"]) jsonRoot["logging"]["log_file"] = log["log_file"].as<std::string>();
            if (log["console_logging"]) jsonRoot["logging"]["console_logging"] = log["console_logging"].as<bool>();
            if (log["json"]) jsonRoot["logging"]["json"] = log["json"].as<bool>();
        }
        return parseJson(jsonRoot);
    } catch (...) {
        return false;
    }
#else
    // No yaml-cpp: try a very naive detection and fail
    (void)yaml_config;
    return false;
#endif
}

// Minimal INI parsing for flat sections using regex; robust parser can be added later
bool TftpConfig::parseIni(const std::string& ini_config) {
    try {
        std::istringstream ss(ini_config);
        std::string line, section;
        std::map<std::string, std::map<std::string, std::string>> kv;
        std::regex sectionRe("^\\s*\\[(.+)\\]\\s*$");
        std::regex kvRe("^\\s*([^=:#]+?)\\s*[:=]\\s*(.+?)\\s*$");
        while (std::getline(ss, line)) {
            std::smatch m;
            if (std::regex_match(line, m, sectionRe)) {
                section = m[1];
                continue;
            }
            if (std::regex_match(line, m, kvRe)) {
                kv[section][m[1]] = m[2];
            }
        }
        Json::Value root;
        auto putStr = [&](const std::string& sec, const std::string& key, const std::string& outKey) {
            if (kv.count(sec) && kv[sec].count(key)) root[sec][outKey] = kv[sec][key];
        };
        auto putBool = [&](const std::string& sec, const std::string& key, const std::string& outKey) {
            if (kv.count(sec) && kv[sec].count(key)) root[sec][outKey] = (kv[sec][key] == "true" || kv[sec][key] == "1");
        };
        auto putUInt = [&](const std::string& sec, const std::string& key, const std::string& outKey) {
            if (kv.count(sec) && kv[sec].count(key)) root[sec][outKey] = (Json::UInt)std::stoul(kv[sec][key]);
        };
        // Map common sections/keys
        putStr("network","listen_address","listen_address");
        putUInt("network","listen_port","listen_port");
        putBool("network","ipv6_enabled","ipv6_enabled");
        putStr("filesystem","root_directory","root_directory");
        // allowed_directories: comma-separated
        if (kv.count("filesystem") && kv["filesystem"].count("allowed_directories")) {
            std::stringstream dirs(kv["filesystem"]["allowed_directories"]);
            std::string item;
            while (std::getline(dirs, item, ',')) {
                root["filesystem"]["allowed_directories"].append(item);
            }
        }
        putBool("security","read_enabled","read_enabled");
        putBool("security","write_enabled","write_enabled");
        if (kv.count("security") && kv["security"].count("max_file_size")) {
            root["security"]["max_file_size"] = (Json::UInt64)std::stoull(kv["security"]["max_file_size"]);
        }
        putBool("security","overwrite_protection","overwrite_protection");
        putUInt("performance","block_size","block_size");
        putUInt("performance","timeout","timeout");
        putUInt("performance","window_size","window_size");
        putStr("logging","level","level");
        putStr("logging","log_file","log_file");
        putBool("logging","console_logging","console_logging");
        putBool("logging","json","json");
        return parseJson(root);
    } catch (...) {
        return false;
    }
}

bool TftpConfig::saveToFile(const std::string& config_file) const {
    // For now, just return true as we haven't implemented JSON serialization yet
    // This will be implemented when we add the actual JSON serialization functionality
    return true;
}

std::string TftpConfig::toJson() const {
    // For now, return a basic JSON string
    // This will be implemented when we add the actual JSON serialization functionality
    return "{}";
}

bool TftpConfig::validate() const {
    // Basic validation
    if (listen_port_ == 0 || listen_port_ > 65535) {
        return false;
    }
    
    if (root_directory_.empty()) {
        return false;
    }
    
    if (block_size_ < 8 || block_size_ > 65464) {
        return false;
    }
    
    if (timeout_ == 0 || timeout_ > 255) {
        return false;
    }
    
    return true;
}

// Network configuration
void TftpConfig::setListenAddress(const std::string& address) {
    listen_address_ = address;
}

std::string TftpConfig::getListenAddress() const {
    return listen_address_;
}

void TftpConfig::setListenPort(port_t port) {
    listen_port_ = port;
}

port_t TftpConfig::getListenPort() const {
    return listen_port_;
}

void TftpConfig::setIpv6Enabled(bool enable) {
    ipv6_enabled_ = enable;
}

bool TftpConfig::isIpv6Enabled() const {
    return ipv6_enabled_;
}

// File system configuration
void TftpConfig::setRootDirectory(const std::string& root_dir) {
    root_directory_ = root_dir;
}

std::string TftpConfig::getRootDirectory() const {
    return root_directory_;
}

void TftpConfig::setAllowedDirectories(const std::vector<std::string>& dirs) {
    allowed_directories_ = dirs;
}

std::vector<std::string> TftpConfig::getAllowedDirectories() const {
    return allowed_directories_;
}

bool TftpConfig::isDirectoryAllowed(const std::string& dir) const {
    if (allowed_directories_.empty()) {
        return true; // If no restrictions, allow all
    }
    
    for (const auto& allowed_dir : allowed_directories_) {
        if (dir.find(allowed_dir) == 0) {
            return true;
        }
    }
    
    return false;
}

// Security configuration
void TftpConfig::setReadEnabled(bool enable) {
    read_enabled_ = enable;
}

bool TftpConfig::isReadEnabled() const {
    return read_enabled_;
}

void TftpConfig::setWriteEnabled(bool enable) {
    write_enabled_ = enable;
}

bool TftpConfig::isWriteEnabled() const {
    return write_enabled_;
}

void TftpConfig::setMaxFileSize(size_t max_size) {
    max_file_size_ = max_size;
}

size_t TftpConfig::getMaxFileSize() const {
    return max_file_size_;
}

void TftpConfig::setOverwriteProtection(bool enable) {
    overwrite_protection_ = enable;
}

bool TftpConfig::isOverwriteProtectionEnabled() const {
    return overwrite_protection_;
}

// Performance configuration
void TftpConfig::setBlockSize(uint16_t block_size) {
    block_size_ = block_size;
}

uint16_t TftpConfig::getBlockSize() const {
    return block_size_;
}

void TftpConfig::setTimeout(uint16_t timeout) {
    timeout_ = timeout;
}

uint16_t TftpConfig::getTimeout() const {
    return timeout_;
}

void TftpConfig::setWindowSize(uint16_t window_size) {
    window_size_ = window_size;
}

uint16_t TftpConfig::getWindowSize() const {
    return window_size_;
}

// Logging configuration
void TftpConfig::setLogLevel(LogLevel level) {
    log_level_ = level;
}

LogLevel TftpConfig::getLogLevel() const {
    return log_level_;
}

void TftpConfig::setLogFile(const std::string& log_file) {
    log_file_ = log_file;
}

std::string TftpConfig::getLogFile() const {
    return log_file_;
}

void TftpConfig::setConsoleLogging(bool enable) {
    console_logging_ = enable;
}

bool TftpConfig::isConsoleLoggingEnabled() const {
    return console_logging_;
}

void TftpConfig::setJsonLogging(bool enable) {
    json_logging_ = enable;
}

bool TftpConfig::isJsonLoggingEnabled() const {
    return json_logging_;
}

bool TftpConfig::parseJson(const Json::Value& root) {
    try {
        // Parse network settings
        if (root.isMember("network")) {
            const Json::Value& network = root["network"];
            
            if (network.isMember("listen_address")) {
                listen_address_ = network["listen_address"].asString();
            }
            
            if (network.isMember("listen_port")) {
                listen_port_ = static_cast<port_t>(network["listen_port"].asUInt());
            }
            
            if (network.isMember("ipv6_enabled")) {
                ipv6_enabled_ = network["ipv6_enabled"].asBool();
            }
        }
        
        // Parse filesystem settings
        if (root.isMember("filesystem")) {
            const Json::Value& filesystem = root["filesystem"];
            
            if (filesystem.isMember("root_directory")) {
                root_directory_ = filesystem["root_directory"].asString();
            }
            
            if (filesystem.isMember("allowed_directories")) {
                allowed_directories_.clear();
                const Json::Value& dirs = filesystem["allowed_directories"];
                for (const auto& dir : dirs) {
                    allowed_directories_.push_back(dir.asString());
                }
            }
        }
        
        // Parse security settings
        if (root.isMember("security")) {
            const Json::Value& security = root["security"];
            
            if (security.isMember("read_enabled")) {
                read_enabled_ = security["read_enabled"].asBool();
            }
            
            if (security.isMember("write_enabled")) {
                write_enabled_ = security["write_enabled"].asBool();
            }
            
            if (security.isMember("max_file_size")) {
                max_file_size_ = static_cast<size_t>(security["max_file_size"].asUInt64());
            }
            
            if (security.isMember("overwrite_protection")) {
                overwrite_protection_ = security["overwrite_protection"].asBool();
            }
        }
        
        // Parse performance settings
        if (root.isMember("performance")) {
            const Json::Value& performance = root["performance"];
            
            if (performance.isMember("block_size")) {
                block_size_ = static_cast<uint16_t>(performance["block_size"].asUInt());
            }
            
            if (performance.isMember("timeout")) {
                timeout_ = static_cast<uint16_t>(performance["timeout"].asUInt());
            }
            
            if (performance.isMember("window_size")) {
                window_size_ = static_cast<uint16_t>(performance["window_size"].asUInt());
            }
        }
        
        // Parse logging settings
        if (root.isMember("logging")) {
            const Json::Value& logging = root["logging"];
            
            if (logging.isMember("level")) {
                std::string level_str = logging["level"].asString();
                if (level_str == "DEBUG") {
                    log_level_ = LogLevel::DEBUG;
                } else if (level_str == "INFO") {
                    log_level_ = LogLevel::INFO;
                } else if (level_str == "WARNING") {
                    log_level_ = LogLevel::WARNING;
                } else if (level_str == "ERROR") {
                    log_level_ = LogLevel::ERROR;
                } else if (level_str == "FATAL") {
                    log_level_ = LogLevel::FATAL;
                }
            }
            
            if (logging.isMember("log_file")) {
                log_file_ = logging["log_file"].asString();
            }
            
            if (logging.isMember("console_logging")) {
                console_logging_ = logging["console_logging"].asBool();
            }
            if (logging.isMember("json")) {
                json_logging_ = logging["json"].asBool();
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace simple_tftpd
