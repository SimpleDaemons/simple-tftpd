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

#include "simple-tftpd/config/config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <json/json.h>

namespace simple_tftpd {

namespace {

std::string normalizeExtension(const std::string& ext) {
    std::string normalized = ext;
    if (!normalized.empty() && normalized.front() == '.') {
        normalized.erase(normalized.begin());
    }
    
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    return normalized;
}

} // namespace

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
    allowed_extensions_.clear();
    
    // Security settings
    read_enabled_ = true;
    write_enabled_ = false;
    max_file_size_ = 100 * 1024 * 1024; // 100MB
    overwrite_protection_ = true;
    allowed_clients_.clear();
    
    // Performance settings
    block_size_ = 512;
    timeout_ = 5;
    window_size_ = 1;
    max_retries_ = 5;
    
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
    
    return loadFromJson(buffer.str());
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

bool TftpConfig::saveToFile(const std::string& config_file) const {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        return false;
    }
    
    file << toJson();
    return static_cast<bool>(file);
}

std::string TftpConfig::toJson() const {
    Json::Value root;
    
    auto& network = root["network"];
    network["listen_address"] = listen_address_;
    network["listen_port"] = listen_port_;
    network["ipv6_enabled"] = ipv6_enabled_;
    
    auto& filesystem = root["filesystem"];
    filesystem["root_directory"] = root_directory_;
    for (const auto& dir : allowed_directories_) {
        filesystem["allowed_directories"].append(dir);
    }
    
    auto& security = root["security"];
    security["read_enabled"] = read_enabled_;
    security["write_enabled"] = write_enabled_;
    security["max_file_size"] = static_cast<Json::UInt64>(max_file_size_);
    security["overwrite_protection"] = overwrite_protection_;
    for (const auto& ext : allowed_extensions_) {
        security["allowed_extensions"].append(ext);
    }
    for (const auto& client : allowed_clients_) {
        security["allowed_clients"].append(client);
    }
    
    auto& performance = root["performance"];
    performance["block_size"] = block_size_;
    performance["timeout"] = timeout_;
    performance["window_size"] = window_size_;
    performance["max_retries"] = max_retries_;
    
    auto& logging = root["logging"];
    logging["level"] = Logger::levelToString(log_level_);
    logging["log_file"] = log_file_;
    logging["console_logging"] = console_logging_;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    return Json::writeString(builder, root);
}

bool TftpConfig::validate() const {
    // Basic validation
    // port_t is uint16_t, so it can only be 0-65535, no need to check upper bound
    if (listen_port_ == 0) {
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
    
    if (max_retries_ == 0 || max_retries_ > 25) {
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

void TftpConfig::setAllowedExtensions(const std::vector<std::string>& extensions) {
    allowed_extensions_.clear();
    allowed_extensions_.reserve(extensions.size());
    for (const auto& ext : extensions) {
        auto normalized = normalizeExtension(ext);
        if (!normalized.empty()) {
            allowed_extensions_.push_back(normalized);
        }
    }
}

std::vector<std::string> TftpConfig::getAllowedExtensions() const {
    return allowed_extensions_;
}

bool TftpConfig::isExtensionAllowed(const std::string& extension) const {
    if (allowed_extensions_.empty()) {
        return true;
    }
    
    auto normalized = normalizeExtension(extension);
    return std::find(allowed_extensions_.begin(), allowed_extensions_.end(), normalized) != allowed_extensions_.end();
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

void TftpConfig::setAllowedClients(const std::vector<std::string>& clients) {
    allowed_clients_ = clients;
}

std::vector<std::string> TftpConfig::getAllowedClients() const {
    return allowed_clients_;
}

bool TftpConfig::isClientAllowed(const std::string& address) const {
    if (allowed_clients_.empty()) {
        return true;
    }
    
    for (const auto& allowed : allowed_clients_) {
        if (allowed == "*" || allowed == address) {
            return true;
        }
    }
    
    return false;
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

void TftpConfig::setMaxRetries(uint16_t retries) {
    max_retries_ = retries == 0 ? 1 : retries;
}

uint16_t TftpConfig::getMaxRetries() const {
    return max_retries_;
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
            
            if (security.isMember("allowed_extensions")) {
                std::vector<std::string> extensions;
                const auto& exts = security["allowed_extensions"];
                for (const auto& ext : exts) {
                    extensions.push_back(ext.asString());
                }
                setAllowedExtensions(extensions);
            }
            
            if (security.isMember("allowed_clients")) {
                allowed_clients_.clear();
                const auto& clients = security["allowed_clients"];
                for (const auto& client : clients) {
                    allowed_clients_.push_back(client.asString());
                }
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
            
            if (performance.isMember("max_retries")) {
                max_retries_ = static_cast<uint16_t>(performance["max_retries"].asUInt());
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
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace simple_tftpd
