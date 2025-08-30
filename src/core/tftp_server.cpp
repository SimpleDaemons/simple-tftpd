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

#include "simple_tftpd/tftp_server.hpp"
#include <iostream>
#include <sstream>

namespace simple_tftpd {

TftpServer::TftpServer(std::shared_ptr<TftpConfig> config, std::shared_ptr<Logger> logger)
    : config_(config),
      logger_(logger),
      running_(false),
      shutdown_requested_(false),
      server_socket_(INVALID_SOCKET_VALUE),
      listen_address_(config->getListenAddress()),
      listen_port_(config->getListenPort()),
      ipv6_enabled_(config->isIpv6Enabled()) {
    
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

bool TftpServer::reloadConfig() {
    logEvent(LogLevel::INFO, "Reloading configuration");
    // Basic implementation - just log for now
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
    
    while (running_.load() && !shutdown_requested_.load()) {
        // Basic listener - just sleep for now
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    // Basic socket initialization - just return true for now
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
    // Basic packet handling - just log for now
    logEvent(LogLevel::DEBUG, "Handling packet from " + sender_addr + ":" + std::to_string(sender_port));
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
    return port > 0 && port <= 65535;
}

bool TftpServer::setSocketOptions() {
    // Basic socket options - just return true for now
    return true;
}

bool TftpServer::bindSocket() {
    // Basic socket binding - just return true for now
    return true;
}

bool TftpServer::setNonBlocking() {
    // Basic non-blocking setup - just return true for now
    return true;
}

} // namespace simple_tftpd
