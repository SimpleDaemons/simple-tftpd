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

#pragma once

#include "simple-tftpd/utils/platform.hpp"
#include "simple-tftpd/utils/logger.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace Json {
class Value;
}

namespace simple_tftpd {

/**
 * @brief TFTP server configuration class
 * 
 * Manages all configuration options for the TFTP daemon including
 * network settings, security options, and file access controls.
 */
class TftpConfig {
public:
    /**
     * @brief Constructor
     */
    TftpConfig();
    
    /**
     * @brief Destructor
     */
    ~TftpConfig();
    
    /**
     * @brief Load configuration from file
     * @param config_file Path to configuration file
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& config_file);
    
    /**
     * @brief Load configuration from JSON string
     * @param json_config JSON configuration string
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromJson(const std::string& json_config);
    
    /**
     * @brief Save configuration to file
     * @param config_file Path to configuration file
     * @return true if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& config_file) const;
    
    /**
     * @brief Get configuration as JSON string
     * @return JSON configuration string
     */
    std::string toJson() const;
    
    /**
     * @brief Validate configuration
     * @return true if valid, false otherwise
     */
    bool validate() const;
    
    // Network configuration
    /**
     * @brief Set listening address
     * @param address IP address to listen on
     */
    void setListenAddress(const std::string& address);
    
    /**
     * @brief Get listening address
     * @return Listening address
     */
    std::string getListenAddress() const;
    
    /**
     * @brief Set listening port
     * @param port Port to listen on
     */
    void setListenPort(port_t port);
    
    /**
     * @brief Get listening port
     * @return Listening port
     */
    port_t getListenPort() const;
    
    /**
     * @brief Enable/disable IPv6
     * @param enable Whether to enable IPv6
     */
    void setIpv6Enabled(bool enable);
    
    /**
     * @brief Check if IPv6 is enabled
     * @return true if IPv6 is enabled
     */
    bool isIpv6Enabled() const;
    
    // File system configuration
    /**
     * @brief Set root directory
     * @param root_dir Root directory for file operations
     */
    void setRootDirectory(const std::string& root_dir);
    
    /**
     * @brief Get root directory
     * @return Root directory
     */
    std::string getRootDirectory() const;
    
    /**
     * @brief Set allowed directories
     * @param dirs Vector of allowed directory paths
     */
    void setAllowedDirectories(const std::vector<std::string>& dirs);
    
    /**
     * @brief Get allowed directories
     * @return Vector of allowed directory paths
     */
    std::vector<std::string> getAllowedDirectories() const;
    
    /**
     * @brief Check if directory is allowed
     * @param dir Directory path to check
     * @return true if directory is allowed
     */
    bool isDirectoryAllowed(const std::string& dir) const;
    
    /**
     * @brief Set allowed file extensions
     * @param extensions List of extensions (without dot)
     */
    void setAllowedExtensions(const std::vector<std::string>& extensions);
    
    /**
     * @brief Get allowed file extensions
     * @return Vector of extensions
     */
    std::vector<std::string> getAllowedExtensions() const;
    
    /**
     * @brief Check if extension is allowed
     * @param extension Extension without dot
     * @return true if allowed
     */
    bool isExtensionAllowed(const std::string& extension) const;
    
    // Security configuration
    /**
     * @brief Enable/disable read operations
     * @param enable Whether to enable read operations
     */
    void setReadEnabled(bool enable);
    
    /**
     * @brief Check if read operations are enabled
     * @return true if read operations are enabled
     */
    bool isReadEnabled() const;
    
    /**
     * @brief Enable/disable write operations
     * @param enable Whether to enable write operations
     */
    void setWriteEnabled(bool enable);
    
    /**
     * @brief Check if write operations are enabled
     * @return true if write operations are enabled
     */
    bool isWriteEnabled() const;
    
    /**
     * @brief Set maximum file size
     * @param max_size Maximum file size in bytes
     */
    void setMaxFileSize(size_t max_size);
    
    /**
     * @brief Get maximum file size
     * @return Maximum file size in bytes
     */
    size_t getMaxFileSize() const;
    
    /**
     * @brief Enable/disable overwrite protection
     * @param enable Whether to enable overwrite protection
     */
    void setOverwriteProtection(bool enable);
    
    /**
     * @brief Check if overwrite protection is enabled
     * @return true if overwrite protection is enabled
     */
    bool isOverwriteProtectionEnabled() const;
    
    /**
     * @brief Set allowed client addresses
     * @param clients List of IPv4/IPv6 addresses
     */
    void setAllowedClients(const std::vector<std::string>& clients);
    
    /**
     * @brief Get allowed client addresses
     * @return Vector of address strings
     */
    std::vector<std::string> getAllowedClients() const;
    
    /**
     * @brief Check if a client address is allowed
     * @param address Address to check
     * @return true if permitted
     */
    bool isClientAllowed(const std::string& address) const;
    
    // Performance configuration
    /**
     * @brief Set block size
     * @param block_size Block size for transfers
     */
    void setBlockSize(uint16_t block_size);
    
    /**
     * @brief Get block size
     * @return Block size for transfers
     */
    uint16_t getBlockSize() const;
    
    /**
     * @brief Set timeout value
     * @param timeout Timeout in seconds
     */
    void setTimeout(uint16_t timeout);
    
    /**
     * @brief Get timeout value
     * @return Timeout in seconds
     */
    uint16_t getTimeout() const;
    
    /**
     * @brief Set window size for windowed transfers
     * @param window_size Window size
     */
    void setWindowSize(uint16_t window_size);
    
    /**
     * @brief Get window size
     * @return Window size
     */
    uint16_t getWindowSize() const;
    
    /**
     * @brief Set max retry attempts for packet retransmission
     * @param retries Number of retries
     */
    void setMaxRetries(uint16_t retries);
    
    /**
     * @brief Get max retry attempts
     * @return Max retry attempts
     */
    uint16_t getMaxRetries() const;
    
    // Logging configuration
    /**
     * @brief Set log level
     * @param level Log level
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief Get log level
     * @return Log level
     */
    LogLevel getLogLevel() const;
    
    /**
     * @brief Set log file path
     * @param log_file Path to log file
     */
    void setLogFile(const std::string& log_file);
    
    /**
     * @brief Get log file path
     * @return Path to log file
     */
    std::string getLogFile() const;
    
    /**
     * @brief Enable/disable console logging
     * @param enable Whether to enable console logging
     */
    void setConsoleLogging(bool enable);
    
    /**
     * @brief Check if console logging is enabled
     * @return true if console logging is enabled
     */
    bool isConsoleLoggingEnabled() const;

private:
    // Network settings
    std::string listen_address_;
    port_t listen_port_;
    bool ipv6_enabled_;
    
    // File system settings
    std::string root_directory_;
    std::vector<std::string> allowed_directories_;
    std::vector<std::string> allowed_extensions_;
    
    // Security settings
    bool read_enabled_;
    bool write_enabled_;
    size_t max_file_size_;
    bool overwrite_protection_;
    std::vector<std::string> allowed_clients_;
    
    // Performance settings
    uint16_t block_size_;
    uint16_t timeout_;
    uint16_t window_size_;
    uint16_t max_retries_;
    
    // Logging settings
    LogLevel log_level_;
    std::string log_file_;
    bool console_logging_;
    
    /**
     * @brief Set default values
     */
    void setDefaults();
    
    /**
     * @brief Parse JSON configuration
     * @param json_config JSON configuration string
     * @return true if parsed successfully, false otherwise
     */
    bool parseJson(const Json::Value& json_config);
};

} // namespace simple_tftpd
