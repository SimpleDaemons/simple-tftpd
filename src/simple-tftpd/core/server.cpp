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

#include "simple-tftpd/core/tftp/server.hpp"
#include "simple-tftpd/core/tftp/monitoring.hpp"
#include <iostream>
#include <sstream>
#include <cstring>

namespace simple_tftpd {

TftpServer::TftpServer(std::shared_ptr<TftpConfig> config, std::shared_ptr<Logger> logger)
    : config_(config),
      logger_(logger),
      running_(false),
      shutdown_requested_(false),
      server_socket_(INVALID_SOCKET_VALUE),
      listen_address_(config->getListenAddress()),
      listen_port_(config->getListenPort()),
      ipv6_enabled_(config->isIpv6Enabled()),
      config_file_path_(""),
      monitoring_(std::make_unique<Monitoring>()) {
    
    stats_.start_time = std::chrono::steady_clock::now();
}

TftpServer::~TftpServer() {
    stop();
}

bool TftpServer::start() {
    if (running_.load()) {
        return false;
    }
    
    logEvent(LogLevel::INFO, "Starting TFTP server");
    
    // Initialize socket
    if (!initializeSocket()) {
        logEvent(LogLevel::ERROR, "Failed to initialize socket");
        return false;
    }
    
    // Set socket options
    if (!setSocketOptions()) {
        logEvent(LogLevel::ERROR, "Failed to set socket options");
        closeSocket();
        return false;
    }
    
    // Bind socket
    if (!bindSocket()) {
        logEvent(LogLevel::ERROR, "Failed to bind socket");
        closeSocket();
        return false;
    }
    
    // Set non-blocking mode
    if (!setNonBlocking()) {
        logEvent(LogLevel::ERROR, "Failed to set non-blocking mode");
        closeSocket();
        return false;
    }
    
    running_.store(true);
    
    // Start listener thread
    listener_thread_ = std::thread(&TftpServer::listenerThread, this);
    
    // Start cleanup thread
    cleanup_thread_ = std::thread(&TftpServer::cleanupThread, this);
    
    logEvent(LogLevel::INFO, "TFTP server started successfully");
    logEvent(LogLevel::INFO, "Listening on " + listen_address_ + ":" + std::to_string(listen_port_));
    
    return true;
}

void TftpServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    logEvent(LogLevel::INFO, "Stopping TFTP server");
    
    shutdown_requested_.store(true);
    running_.store(false);
    
    // Close socket
    closeSocket();
    
    // Wait for threads to finish
    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    // Close all connections
    closeAllConnections();
    
    logEvent(LogLevel::INFO, "TFTP server stopped");
}

bool TftpServer::isRunning() const {
    return running_.load();
}

std::string TftpServer::getStatus() const {
    std::stringstream ss;
    ss << "TFTP Server Status:" << std::endl;
    ss << "  Running: " << (running_.load() ? "Yes" : "No") << std::endl;
    ss << "  Listen Address: " << listen_address_ << std::endl;
    ss << "  Listen Port: " << listen_port_ << std::endl;
    ss << "  IPv6 Enabled: " << (ipv6_enabled_ ? "Yes" : "No") << std::endl;
    ss << "  Active Connections: " << getActiveConnectionCount() << std::endl;
    ss << "  Uptime: " << getUptime().count() << " seconds" << std::endl;
    
    return ss.str();
}

size_t TftpServer::getActiveConnectionCount() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    return connections_.size();
}

std::shared_ptr<TftpConfig> TftpServer::getConfig() const {
    return config_;
}

std::shared_ptr<Logger> TftpServer::getLogger() const {
    return logger_;
}

void TftpServer::setConfigFile(const std::string& config_file) {
    config_file_path_ = config_file;
}

bool TftpServer::reloadConfig(const std::string& config_file) {
    std::string file_to_load = config_file.empty() ? config_file_path_ : config_file;
    
    if (file_to_load.empty()) {
        logEvent(LogLevel::WARNING, "No configuration file path set for reload");
        return false;
    }
    
    logEvent(LogLevel::INFO, "Reloading configuration from: " + file_to_load);
    
    // Create a new config object and load from file
    auto new_config = std::make_shared<TftpConfig>();
    if (!new_config->loadFromFile(file_to_load)) {
        logEvent(LogLevel::ERROR, "Failed to reload configuration from: " + file_to_load);
        return false;
    }
    
    // Store old log file for comparison before replacing config
    std::string old_log_file = config_->getLogFile();
    
    // Validate critical settings that can't change while running
    if (new_config->getListenAddress() != listen_address_ ||
        new_config->getListenPort() != listen_port_ ||
        new_config->isIpv6Enabled() != ipv6_enabled_) {
        logEvent(LogLevel::WARNING, "Network settings changed but cannot be applied without restart");
        logEvent(LogLevel::WARNING, "Please restart the server to apply network configuration changes");
    }
    
    // Update config (thread-safe)
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        config_ = new_config;
        
        // Update active connections with new config values
        for (auto& pair : connections_) {
            auto connection = pair.second;
            if (connection && connection->isActive()) {
                // Update connection with new config (connections will use new config for new transfers)
                // Note: Existing transfers continue with their original config
            }
        }
    }
    
    // Update logger settings if they changed
    if (logger_) {
        logger_->setLevel(new_config->getLogLevel());
        logger_->setConsoleOutput(new_config->isConsoleLoggingEnabled());
        if (!new_config->getLogFile().empty()) {
            // Note: Changing log file would require recreating logger
            // For now, just log a warning
            if (new_config->getLogFile() != old_log_file) {
                logEvent(LogLevel::INFO, "Log file setting changed (requires restart to take effect)");
            }
        }
    }
    
    logEvent(LogLevel::INFO, "Configuration reloaded successfully");
    return true;
}

TftpServerStats TftpServer::getStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void TftpServer::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = TftpServerStats();
    stats_.start_time = std::chrono::steady_clock::now();
}

std::chrono::seconds TftpServer::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - stats_.start_time);
}

HealthCheckResult TftpServer::performHealthCheck() const {
    if (!monitoring_) {
        HealthCheckResult result;
        result.status = HealthStatus::UNHEALTHY;
        result.message = "Monitoring not initialized";
        return result;
    }
    return monitoring_->performHealthCheck();
}

ServerMetrics TftpServer::getMetrics() const {
    if (!monitoring_) {
        return ServerMetrics();
    }
    
    // Update monitoring with current server stats
    std::lock_guard<std::mutex> lock(stats_mutex_);
    monitoring_->updateActiveConnections(stats_.active_connections);
    
    return monitoring_->getMetrics();
}

std::string TftpServer::getMetricsJson() const {
    if (!monitoring_) {
        return "{\"error\": \"Monitoring not initialized\"}";
    }
    
    // Update monitoring with current server stats
    std::lock_guard<std::mutex> lock(stats_mutex_);
    monitoring_->updateActiveConnections(stats_.active_connections);
    
    return monitoring_->getMetricsJson();
}

std::string TftpServer::getHealthCheckJson() const {
    if (!monitoring_) {
        return "{\"status\": \"unhealthy\", \"message\": \"Monitoring not initialized\"}";
    }
    return monitoring_->getHealthCheckJson();
}

void TftpServer::setConnectionCallback(std::function<void(TftpConnectionState, const std::string&)> callback) {
    connection_callback_ = callback;
}

void TftpServer::setServerCallback(std::function<void(const std::string&, const std::string&)> callback) {
    server_callback_ = callback;
}

void TftpServer::broadcastMessage(const std::string& message) {
    logEvent(LogLevel::INFO, "Broadcasting message: " + message);
    // Basic implementation - just log for now
}

bool TftpServer::closeConnection(const std::string& client_addr, port_t client_port) {
    std::string key = generateConnectionKey(client_addr, client_port);
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(key);
    if (it != connections_.end()) {
        it->second->stop();
        connections_.erase(it);
        return true;
    }
    
    return false;
}

void TftpServer::closeAllConnections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (auto& connection : connections_) {
        connection.second->stop();
    }
    connections_.clear();
}

std::string TftpServer::getConnectionInfo(const std::string& client_addr, port_t client_port) const {
    std::string key = generateConnectionKey(client_addr, client_port);
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(key);
    if (it != connections_.end()) {
        auto conn = it->second;
        std::stringstream ss;
        ss << "Connection: " << client_addr << ":" << client_port << std::endl;
        ss << "  State: " << static_cast<int>(conn->getState()) << std::endl;
        ss << "  Filename: " << conn->getFilename() << std::endl;
        ss << "  Bytes Transferred: " << conn->getBytesTransferred() << std::endl;
        ss << "  Duration: " << conn->getDuration().count() << " seconds" << std::endl;
        return ss.str();
    }
    
    return "Connection not found";
}

std::vector<std::string> TftpServer::listConnections() const {
    std::vector<std::string> result;
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (const auto& connection : connections_) {
        result.push_back(getConnectionInfo(connection.second->getClientAddress(), 
                                         connection.second->getClientPort()));
    }
    
    return result;
}

void TftpServer::listenerThread() {
    logEvent(LogLevel::INFO, "Listener thread started");
    
    uint8_t buffer[TFTP_MAX_PACKET_SIZE];
    
    while (running_.load() && !shutdown_requested_.load()) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        ssize_t bytes_received = recvfrom(server_socket_, 
                                        reinterpret_cast<char*>(buffer), 
                                        sizeof(buffer), 
                                        0,
                                        reinterpret_cast<struct sockaddr*>(&client_addr),
                                        &client_addr_len);
        
        if (bytes_received > 0) {
            // Convert client address to string
            std::string client_addr_str;
            port_t client_port = 0;
            
            if (client_addr.ss_family == AF_INET) {
                struct sockaddr_in* addr4 = reinterpret_cast<struct sockaddr_in*>(&client_addr);
                char addr_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr4->sin_addr, addr_str, INET_ADDRSTRLEN);
                client_addr_str = addr_str;
                client_port = ntohs(addr4->sin_port);
            } else if (client_addr.ss_family == AF_INET6) {
                struct sockaddr_in6* addr6 = reinterpret_cast<struct sockaddr_in6*>(&client_addr);
                char addr_str[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &addr6->sin6_addr, addr_str, INET6_ADDRSTRLEN);
                client_addr_str = addr_str;
                client_port = ntohs(addr6->sin6_port);
            }
            
            if (config_ && !config_->isClientAllowed(client_addr_str)) {
                logEvent(LogLevel::WARNING, "Rejected packet from unauthorized client " + client_addr_str);
                continue;
            }
            
            // Handle the received packet
            handlePacket(buffer, static_cast<size_t>(bytes_received), client_addr_str, client_port);
        } else if (bytes_received < 0) {
#ifdef PLATFORM_WINDOWS
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAETIMEDOUT) {
                logEvent(LogLevel::ERROR, "Socket receive error: " + std::to_string(error));
            }
#else
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                logEvent(LogLevel::ERROR, "Socket receive error: " + std::to_string(errno));
            }
#endif
        }
        
        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    logEvent(LogLevel::INFO, "Listener thread stopped");
}

void TftpServer::cleanupThread() {
    logEvent(LogLevel::INFO, "Cleanup thread started");
    
    while (running_.load() && !shutdown_requested_.load()) {
        // Basic cleanup - just sleep for now
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        cleanupInactiveConnections();
    }
    
    logEvent(LogLevel::INFO, "Cleanup thread stopped");
}

bool TftpServer::initializeSocket() {
#ifdef PLATFORM_WINDOWS
    // Initialize Winsock on Windows
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        logEvent(LogLevel::ERROR, "WSAStartup failed: " + std::to_string(result));
        return false;
    }
#endif

    // Create UDP socket
    int family = ipv6_enabled_ ? AF_INET6 : AF_INET;
    server_socket_ = socket(family, SOCK_DGRAM, IPPROTO_UDP);
    
    if (server_socket_ == INVALID_SOCKET_VALUE) {
        logEvent(LogLevel::ERROR, "Failed to create socket: " + std::to_string(SOCKET_ERROR_CODE));
        return false;
    }
    
    return true;
}

void TftpServer::closeSocket() {
    if (server_socket_ != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
    }
}

bool TftpServer::acceptConnection() {
    // Basic connection acceptance - just return true for now
    return true;
}

void TftpServer::handlePacket(const uint8_t* packet_data,
                             size_t packet_size,
                             const std::string& sender_addr,
                             port_t sender_port) {
    if (packet_size < 2) {
        logEvent(LogLevel::WARNING, "Received packet too small from " + sender_addr + ":" + std::to_string(sender_port));
        return;
    }
    
    // Parse packet type
    uint16_t opcode = (packet_data[0] << 8) | packet_data[1];
    
    std::string connection_key = generateConnectionKey(sender_addr, sender_port);
    
    // Handle different packet types
    switch (static_cast<TftpOpcode>(opcode)) {
        case TftpOpcode::RRQ:
        case TftpOpcode::WRQ: {
            // Create new connection for request
            auto connection = createConnection(sender_addr, sender_port);
            if (connection) {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                connections_[connection_key] = connection;
                connection->start();
                
                // Parse and handle the request
                TftpRequestPacket request(packet_data, packet_size);
                if (request.isValid()) {
                    if (opcode == static_cast<uint16_t>(TftpOpcode::RRQ)) {
                        connection->handleReadRequest(request);
                    } else {
                        connection->handleWriteRequest(request);
                    }
                }
            }
            break;
        }
        
        case TftpOpcode::DATA: {
            // Find existing connection
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(connection_key);
            if (it != connections_.end()) {
                TftpDataPacket data_packet(packet_data, packet_size);
                if (data_packet.isValid()) {
                    it->second->handleDataPacket(data_packet);
                }
            }
            break;
        }
        
        case TftpOpcode::ACK: {
            // Find existing connection
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(connection_key);
            if (it != connections_.end()) {
                TftpAckPacket ack_packet(packet_data, packet_size);
                if (ack_packet.isValid()) {
                    it->second->handleAckPacket(ack_packet);
                }
            }
            break;
        }
        
        case TftpOpcode::ERROR: {
            // Find existing connection
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(connection_key);
            if (it != connections_.end()) {
                TftpErrorPacket error_packet(packet_data, packet_size);
                if (error_packet.isValid()) {
                    it->second->handleErrorPacket(error_packet);
                }
            }
            break;
        }
        
        default:
            logEvent(LogLevel::WARNING, "Unknown packet type " + std::to_string(opcode) + " from " + sender_addr + ":" + std::to_string(sender_port));
            break;
    }
}

std::shared_ptr<TftpConnection> TftpServer::createConnection(const std::string& client_addr, port_t client_port) {
    auto connection = std::make_shared<TftpConnection>(*this, client_addr, client_port, config_, logger_);
    
    if (connection_callback_) {
        connection->setCallback(connection_callback_);
    }
    
    return connection;
}

void TftpServer::removeConnection(const std::string& client_addr, port_t client_port) {
    std::string key = generateConnectionKey(client_addr, client_port);
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(key);
}

void TftpServer::cleanupInactiveConnections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = connections_.begin();
    while (it != connections_.end()) {
        if (!it->second->isActive()) {
            it->second->stop();
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

void TftpServer::updateStats(TftpConnectionState connection_state, size_t bytes_transferred) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    // Basic stats update - just increment counters for now
    stats_.total_connections++;
    
    if (bytes_transferred > 0) {
        stats_.total_bytes_transferred += bytes_transferred;
    }
    
    // Update monitoring
    if (monitoring_) {
        bool success = (connection_state == TftpConnectionState::COMPLETED);
        uint64_t duration_ms = 0; // Could calculate from connection start time
        monitoring_->recordTransfer(bytes_transferred, success, duration_ms);
        monitoring_->recordConnection(success);
        monitoring_->updateActiveConnections(stats_.active_connections);
        
        if (connection_state == TftpConnectionState::ERROR) {
            monitoring_->recordError();
        }
    }
}

void TftpServer::logEvent(LogLevel level, const std::string& message) {
    if (logger_) {
        logger_->log(level, "[Server] " + message);
    }
}

std::string TftpServer::generateConnectionKey(const std::string& client_addr, port_t client_port) {
    return client_addr + ":" + std::to_string(client_port);
}

bool TftpServer::isValidAddress(const std::string& address) const {
    // Basic address validation - just return true for now
    return true;
}

bool TftpServer::isValidPort(port_t port) const {
    // port_t is uint16_t, so it can only be 0-65535, no need to check upper bound
    return port > 0;
}

bool TftpServer::setSocketOptions() {
    // Set socket to reuse address
    int reuse = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, 
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) < 0) {
        logEvent(LogLevel::WARNING, "Failed to set SO_REUSEADDR: " + std::to_string(SOCKET_ERROR_CODE));
    }
    
    // Set socket receive timeout
    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 second timeout
    timeout.tv_usec = 0;
    
    if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, 
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout)) < 0) {
        logEvent(LogLevel::WARNING, "Failed to set receive timeout: " + std::to_string(SOCKET_ERROR_CODE));
    }
    
    // Set socket send timeout
    if (setsockopt(server_socket_, SOL_SOCKET, SO_SNDTIMEO, 
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout)) < 0) {
        logEvent(LogLevel::WARNING, "Failed to set send timeout: " + std::to_string(SOCKET_ERROR_CODE));
    }
    
    return true;
}

bool TftpServer::bindSocket() {
    if (ipv6_enabled_) {
        // Try IPv6 binding first
        struct sockaddr_in6 addr6;
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = htons(listen_port_);
        
        bool ipv6_address_valid = true;
        if (listen_address_ == "0.0.0.0" || listen_address_ == "::") {
            std::memset(&addr6.sin6_addr, 0, sizeof(addr6.sin6_addr));
        } else {
            if (inet_pton(AF_INET6, listen_address_.c_str(), &addr6.sin6_addr) != 1) {
                logEvent(LogLevel::WARNING, "Invalid IPv6 address: " + listen_address_ + ", falling back to IPv4");
                ipv6_address_valid = false;
                ipv6_enabled_ = false;
            }
        }
        
        // Try to bind IPv6 if address is valid
        if (ipv6_address_valid) {
            if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&addr6), sizeof(addr6)) >= 0) {
                logEvent(LogLevel::INFO, "Successfully bound IPv6 socket");
                return true;
            } else {
                // IPv6 binding failed, fall back to IPv4
                logEvent(LogLevel::WARNING, "Failed to bind IPv6 socket (error " + std::to_string(SOCKET_ERROR_CODE) + "), falling back to IPv4");
                ipv6_enabled_ = false;
                // Close IPv6 socket and create IPv4 socket
                closeSocket();
                if (!initializeSocket()) {
                    logEvent(LogLevel::ERROR, "Failed to create IPv4 socket for fallback");
                    return false;
                }
                // Fall through to IPv4 binding
            }
        } else {
            // Invalid IPv6 address, close socket and create IPv4 socket
            closeSocket();
            if (!initializeSocket()) {
                logEvent(LogLevel::ERROR, "Failed to create IPv4 socket for fallback");
                return false;
            }
        }
    }
    
    // IPv4 binding (either explicitly requested or as fallback)
    struct sockaddr_in addr4;
    memset(&addr4, 0, sizeof(addr4));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(listen_port_);
    
    if (listen_address_ == "0.0.0.0" || listen_address_ == "::") {
        addr4.sin_addr.s_addr = INADDR_ANY;
    } else {
        // Convert IPv6 "::" to IPv4 "0.0.0.0" if needed
        std::string bind_addr = listen_address_;
        if (bind_addr == "::") {
            bind_addr = "0.0.0.0";
            addr4.sin_addr.s_addr = INADDR_ANY;
        } else {
            if (inet_pton(AF_INET, bind_addr.c_str(), &addr4.sin_addr) != 1) {
                logEvent(LogLevel::ERROR, "Invalid IPv4 address: " + bind_addr);
                return false;
            }
        }
    }
    
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&addr4), sizeof(addr4)) < 0) {
        int error_code = SOCKET_ERROR_CODE;
        std::string error_msg = "Failed to bind IPv4 socket: " + std::to_string(error_code);
        
        // Provide helpful error messages for common issues
        if (error_code == 13) {  // EACCES - Permission denied
            if (listen_port_ < 1024) {
                error_msg += " (Permission denied - ports below 1024 require root privileges)";
            } else {
                error_msg += " (Permission denied - check firewall/security settings)";
            }
        } else if (error_code == 98 || error_code == 48) {  // EADDRINUSE - Address already in use
            error_msg += " (Port already in use)";
        } else if (error_code == 99) {  // EADDRNOTAVAIL - Address not available
            error_msg += " (Address not available)";
        }
        
        logEvent(LogLevel::ERROR, error_msg);
        return false;
    }
    
    logEvent(LogLevel::INFO, "Successfully bound IPv4 socket");
    return true;
}

bool TftpServer::setNonBlocking() {
#ifdef PLATFORM_WINDOWS
    u_long mode = 1; // 1 to enable non-blocking socket
    if (ioctlsocket(server_socket_, FIONBIO, &mode) != 0) {
        logEvent(LogLevel::ERROR, "Failed to set non-blocking mode: " + std::to_string(SOCKET_ERROR_CODE));
        return false;
    }
#else
    int flags = fcntl(server_socket_, F_GETFL, 0);
    if (flags < 0) {
        logEvent(LogLevel::ERROR, "Failed to get socket flags: " + std::to_string(SOCKET_ERROR_CODE));
        return false;
    }
    
    if (fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK) < 0) {
        logEvent(LogLevel::ERROR, "Failed to set non-blocking mode: " + std::to_string(SOCKET_ERROR_CODE));
        return false;
    }
#endif
    
    return true;
}

bool TftpServer::sendPacket(const uint8_t* packet_data, size_t packet_size, 
                           const std::string& client_addr, port_t client_port) {
    if (!packet_data || packet_size == 0) {
        return false;
    }
    
    struct sockaddr_storage addr;
    socklen_t addr_len;
    
    if (ipv6_enabled_) {
        // IPv6 address
        struct sockaddr_in6* addr6 = reinterpret_cast<struct sockaddr_in6*>(&addr);
        memset(addr6, 0, sizeof(*addr6));
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(client_port);
        
        if (inet_pton(AF_INET6, client_addr.c_str(), &addr6->sin6_addr) != 1) {
            logEvent(LogLevel::ERROR, "Invalid IPv6 client address: " + client_addr);
            return false;
        }
        
        addr_len = sizeof(*addr6);
    } else {
        // IPv4 address
        struct sockaddr_in* addr4 = reinterpret_cast<struct sockaddr_in*>(&addr);
        memset(addr4, 0, sizeof(*addr4));
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(client_port);
        
        if (inet_pton(AF_INET, client_addr.c_str(), &addr4->sin_addr) != 1) {
            logEvent(LogLevel::ERROR, "Invalid IPv4 client address: " + client_addr);
            return false;
        }
        
        addr_len = sizeof(*addr4);
    }
    
    ssize_t bytes_sent = sendto(server_socket_, 
                              reinterpret_cast<const char*>(packet_data), 
                              packet_size, 
                              0,
                              reinterpret_cast<struct sockaddr*>(&addr),
                              addr_len);
    
    if (bytes_sent < 0 || static_cast<size_t>(bytes_sent) != packet_size) {
        logEvent(LogLevel::ERROR, "Failed to send packet to " + client_addr + ":" + std::to_string(client_port) + 
                " - Error: " + std::to_string(SOCKET_ERROR_CODE));
        return false;
    }
    
    return true;
}

} // namespace simple_tftpd
