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
    
    return loadFromJson(buffer.str());
}

bool TftpConfig::loadFromJson(const std::string& json_config) {
    // For now, just return true as we haven't implemented JSON parsing yet
    // This will be implemented when we add the actual JSON parsing functionality
    return true;
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

} // namespace simple_tftpd
