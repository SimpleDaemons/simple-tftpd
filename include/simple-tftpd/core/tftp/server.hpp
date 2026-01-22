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

#include "simple-tftpd/core/utils/platform.hpp"
#include "simple-tftpd/core/tftp/connection.hpp"
#include "simple-tftpd/core/tftp/monitoring.hpp"
#include "simple-tftpd/core/config/config.hpp"
#include "simple-tftpd/core/utils/logger.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace simple_tftpd {

// Forward declarations
class Monitoring;
class ProductionSecurityManager;
struct HealthCheckResult;
struct ServerMetrics;

/**
 * @brief TFTP server statistics
 */
struct TftpServerStats {
    uint64_t total_connections;
    uint64_t active_connections;
    uint64_t total_read_requests;
    uint64_t total_write_requests;
    uint64_t total_bytes_transferred;
    uint64_t total_errors;
    std::chrono::steady_clock::time_point start_time;

    TftpServerStats() : total_connections(0), active_connections(0),
                        total_read_requests(0), total_write_requests(0),
                        total_bytes_transferred(0), total_errors(0) {}
};

/**
 * @brief TFTP server class
 *
 * Main server class that manages the TFTP daemon including:
 * - Listening for incoming connections
 * - Managing multiple TFTP connections
 * - Configuration management
 * - Statistics collection
 * - Graceful shutdown
 */
class TftpServer {
public:
    /**
     * @brief Constructor
     * @param config Server configuration
     * @param logger Logger instance
     */
    TftpServer(std::shared_ptr<TftpConfig> config,
               std::shared_ptr<Logger> logger);

    /**
     * @brief Destructor
     */
    ~TftpServer();

    /**
     * @brief Start the TFTP server
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the TFTP server
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const;

    /**
     * @brief Get server status information
     * @return Status string
     */
    std::string getStatus() const;

    /**
     * @brief Get active connection count
     * @return Number of active connections
     */
    size_t getActiveConnectionCount() const;

    /**
     * @brief Get server configuration
     * @return Server configuration
     */
    std::shared_ptr<TftpConfig> getConfig() const;

    /**
     * @brief Get server logger
     * @return Server logger
     */
    std::shared_ptr<Logger> getLogger() const;

    /**
     * @brief Reload configuration from file
     * @param config_file Optional config file path (uses original if not provided)
     * @return true if reloaded successfully, false otherwise
     */
    bool reloadConfig(const std::string& config_file = "");

    /**
     * @brief Set configuration file path for reloading
     * @param config_file Path to configuration file
     */
    void setConfigFile(const std::string& config_file);

    /**
     * @brief Get server statistics
     * @return Server statistics
     */
    TftpServerStats getStats() const;

    /**
     * @brief Reset server statistics
     */
    void resetStats();

    /**
     * @brief Perform health check
     * @return Health check result
     */
    HealthCheckResult performHealthCheck() const;

    /**
     * @brief Get server metrics
     * @return Server metrics
     */
    ServerMetrics getMetrics() const;

    /**
     * @brief Get metrics as JSON string
     * @return JSON representation of metrics
     */
    std::string getMetricsJson() const;

    /**
     * @brief Get health check as JSON string
     * @return JSON representation of health check
     */
    std::string getHealthCheckJson() const;

    /**
     * @brief Send packet to client
     * @param packet_data Packet data
     * @param packet_size Size of packet data
     * @param client_addr Client address
     * @param client_port Client port
     * @return true if sent successfully, false otherwise
     */
    bool sendPacket(const uint8_t* packet_data, size_t packet_size,
                   const std::string& client_addr, port_t client_port);

    /**
     * @brief Get server uptime
     * @return Server uptime in seconds
     */
    std::chrono::seconds getUptime() const;

    /**
     * @brief Set connection callback
     * @param callback Function to call on connection events
     */
    void setConnectionCallback(std::function<void(TftpConnectionState, const std::string&)> callback);

    /**
     * @brief Set server event callback
     * @param callback Function to call on server events
     */
    void setServerCallback(std::function<void(const std::string&, const std::string&)> callback);

    /**
     * @brief Broadcast message to all active connections
     * @param message Message to broadcast
     */
    void broadcastMessage(const std::string& message);

    /**
     * @brief Close connection by client address and port
     * @param client_addr Client address
     * @param client_port Client port
     * @return true if connection was found and closed, false otherwise
     */
    bool closeConnection(const std::string& client_addr, port_t client_port);

    /**
     * @brief Close all connections
     */
    void closeAllConnections();

    /**
     * @brief Get connection information
     * @param client_addr Client address
     * @param client_port Client port
     * @return Connection information string
     */
    std::string getConnectionInfo(const std::string& client_addr, port_t client_port) const;

    /**
     * @brief List all active connections
     * @return Vector of connection information strings
     */
    std::vector<std::string> listConnections() const;

    /**
     * @brief Set security manager
     * @param security_manager Security manager instance
     */
    void setSecurityManager(std::shared_ptr<ProductionSecurityManager> security_manager);

private:
    std::shared_ptr<TftpConfig> config_;
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<class ProductionSecurityManager> security_manager_; // Optional, for production builds

    std::atomic<bool> running_;
    std::atomic<bool> shutdown_requested_;

    socket_t server_socket_;
    std::string listen_address_;
    port_t listen_port_;
    bool ipv6_enabled_;
    std::string config_file_path_;

    std::thread listener_thread_;
    std::thread cleanup_thread_;

    std::map<std::string, std::shared_ptr<TftpConnection>> connections_;
    mutable std::mutex connections_mutex_;

    TftpServerStats stats_;
    mutable std::mutex stats_mutex_;

    std::unique_ptr<Monitoring> monitoring_;

    std::function<void(TftpConnectionState, const std::string&)> connection_callback_;
    std::function<void(const std::string&, const std::string&)> server_callback_;

    /**
     * @brief Main listener thread
     */
    void listenerThread();

    /**
     * @brief Connection cleanup thread
     */
    void cleanupThread();

    /**
     * @brief Initialize server socket
     * @return true if initialized successfully, false otherwise
     */
    bool initializeSocket();

    /**
     * @brief Close server socket
     */
    void closeSocket();

    /**
     * @brief Accept incoming connection
     * @return true if connection accepted, false otherwise
     */
    bool acceptConnection();

    /**
     * @brief Handle incoming packet
     * @param packet_data Raw packet data
     * @param packet_size Size of packet data
     * @param sender_addr Sender address
     * @param sender_port Sender port
     */
    void handlePacket(const uint8_t* packet_data,
                     size_t packet_size,
                     const std::string& sender_addr,
                     port_t sender_port);

    /**
     * @brief Create new connection
     * @param client_addr Client address
     * @param client_port Client port
     * @return New connection instance
     */
    std::shared_ptr<TftpConnection> createConnection(const std::string& client_addr,
                                                    port_t client_port);

    /**
     * @brief Remove connection
     * @param client_addr Client address
     * @param client_port Client port
     */
    void removeConnection(const std::string& client_addr, port_t client_port);

    /**
     * @brief Clean up inactive connections
     */
    void cleanupInactiveConnections();

    /**
     * @brief Update server statistics
     * @param connection_state Connection state change
     * @param bytes_transferred Bytes transferred (if applicable)
     */
    void updateStats(TftpConnectionState connection_state, size_t bytes_transferred = 0);

    /**
     * @brief Log server event
     * @param level Log level
     * @param message Message to log
     */
    void logEvent(LogLevel level, const std::string& message);

    /**
     * @brief Generate connection key
     * @param client_addr Client address
     * @param client_port Client port
     * @return Connection key string
     */
    static std::string generateConnectionKey(const std::string& client_addr, port_t client_port);

    /**
     * @brief Check if address is valid
     * @param address IP address to check
     * @return true if valid, false otherwise
     */
    bool isValidAddress(const std::string& address) const;

    /**
     * @brief Check if port is valid
     * @param port Port to check
     * @return true if valid, false otherwise
     */
    bool isValidPort(port_t port) const;

    /**
     * @brief Set socket options
     * @return true if options set successfully, false otherwise
     */
    bool setSocketOptions();

    /**
     * @brief Bind socket to address and port
     * @return true if bound successfully, false otherwise
     */
    bool bindSocket();

    /**
     * @brief Set socket to non-blocking mode
     * @return true if set successfully, false otherwise
     */
    bool setNonBlocking();
};

} // namespace simple_tftpd
