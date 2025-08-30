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

#include "simple_tftpd/platform.hpp"
#include "simple_tftpd/tftp_packet.hpp"
#include "simple_tftpd/tftp_config.hpp"
#include "simple_tftpd/logger.hpp"
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

namespace simple_tftpd {

// Forward declarations
class TftpServer;

/**
 * @brief TFTP connection state
 */
enum class TftpConnectionState {
    INITIALIZED,
    CONNECTED,
    TRANSFERRING,
    COMPLETED,
    ERROR,
    CLOSED
};

/**
 * @brief TFTP transfer direction
 */
enum class TftpTransferDirection {
    READ,   // Client reading from server
    WRITE   // Client writing to server
};

/**
 * @brief TFTP connection class
 * 
 * Manages individual TFTP connections including packet handling,
 * file operations, and transfer state management.
 */
class TftpConnection {
public:
    /**
     * @brief Constructor
     * @param server Reference to TFTP server
     * @param client_addr Client address
     * @param client_port Client port
     * @param config Server configuration
     * @param logger Logger instance
     */
    TftpConnection(TftpServer& server,
                  const std::string& client_addr,
                  port_t client_port,
                  std::shared_ptr<TftpConfig> config,
                  std::shared_ptr<Logger> logger);
    
    /**
     * @brief Destructor
     */
    ~TftpConnection();
    
    /**
     * @brief Start connection processing
     * @return true if started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop connection processing
     */
    void stop();
    
    /**
     * @brief Check if connection is active
     * @return true if active, false otherwise
     */
    bool isActive() const;
    
    /**
     * @brief Get connection state
     * @return Current connection state
     */
    TftpConnectionState getState() const;
    
    /**
     * @brief Get client address
     * @return Client IP address
     */
    std::string getClientAddress() const;
    
    /**
     * @brief Get client port
     * @return Client port
     */
    port_t getClientPort() const;
    
    /**
     * @brief Get transfer direction
     * @return Transfer direction
     */
    TftpTransferDirection getTransferDirection() const;
    
    /**
     * @brief Get filename being transferred
     * @return Filename
     */
    std::string getFilename() const;
    
    /**
     * @brief Get bytes transferred
     * @return Number of bytes transferred
     */
    size_t getBytesTransferred() const;
    
    /**
     * @brief Get start time
     * @return Connection start time
     */
    std::chrono::steady_clock::time_point getStartTime() const;
    
    /**
     * @brief Get connection duration
     * @return Connection duration in seconds
     */
    std::chrono::seconds getDuration() const;
    
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
     * @brief Send packet to client
     * @param packet Packet to send
     * @return true if sent successfully, false otherwise
     */
    bool sendPacket(const TftpPacket& packet);
    
    /**
     * @brief Send error packet
     * @param error_code Error code
     * @param error_message Error message
     * @return true if sent successfully, false otherwise
     */
    bool sendError(TftpError error_code, const std::string& error_message);
    
    /**
     * @brief Set connection timeout
     * @param timeout Timeout in seconds
     */
    void setTimeout(std::chrono::seconds timeout);
    
    /**
     * @brief Get connection timeout
     * @return Timeout in seconds
     */
    std::chrono::seconds getTimeout() const;
    
    /**
     * @brief Set connection callback
     * @param callback Function to call on connection events
     */
    void setCallback(std::function<void(TftpConnectionState, const std::string&)> callback);

private:
    TftpServer& server_;
    std::string client_addr_;
    port_t client_port_;
    std::shared_ptr<TftpConfig> config_;
    std::shared_ptr<Logger> logger_;
    
    TftpConnectionState state_;
    TftpTransferDirection direction_;
    std::string filename_;
    TftpMode mode_;
    TftpOptions options_;
    
    size_t bytes_transferred_;
    uint16_t current_block_;
    uint16_t expected_block_;
    
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_activity_;
    std::chrono::seconds timeout_;
    
    std::atomic<bool> active_;
    std::thread worker_thread_;
    std::function<void(TftpConnectionState, const std::string&)> callback_;
    
    // File handling
    std::ifstream read_file_;
    std::ofstream write_file_;
    
    /**
     * @brief Main connection worker thread
     */
    void workerThread();
    
    /**
     * @brief Handle read request
     * @param packet Read request packet
     */
    void handleReadRequest(const TftpRequestPacket& packet);
    
    /**
     * @brief Handle write request
     * @param packet Write request packet
     */
    void handleWriteRequest(const TftpRequestPacket& packet);
    
    /**
     * @brief Handle data packet
     * @param packet Data packet
     */
    void handleDataPacket(const TftpDataPacket& packet);
    
    /**
     * @brief Handle acknowledgment packet
     * @param packet Acknowledgment packet
     */
    void handleAckPacket(const TftpAckPacket& packet);
    
    /**
     * @brief Handle error packet
     * @param packet Error packet
     */
    void handleErrorPacket(const TftpErrorPacket& packet);
    
    /**
     * @brief Process read request
     * @param packet Read request packet
     * @return true if processed successfully, false otherwise
     */
    bool processReadRequest(const TftpRequestPacket& packet);
    
    /**
     * @brief Process write request
     * @param packet Write request packet
     * @return true if processed successfully, false otherwise
     */
    bool processWriteRequest(const TftpRequestPacket& packet);
    
    /**
     * @brief Send data block
     * @param block_number Block number to send
     * @return true if sent successfully, false otherwise
     */
    bool sendDataBlock(uint16_t block_number);
    
    /**
     * @brief Send acknowledgment
     * @param block_number Block number to acknowledge
     * @return true if sent successfully, false otherwise
     */
    bool sendAcknowledgment(uint16_t block_number);
    
    /**
     * @brief Open file for reading
     * @param filename Filename to open
     * @return true if opened successfully, false otherwise
     */
    bool openReadFile(const std::string& filename);
    
    /**
     * @brief Open file for writing
     * @param filename Filename to open
     * @return true if opened successfully, false otherwise
     */
    bool openWriteFile(const std::string& filename);
    
    /**
     * @brief Close files
     */
    void closeFiles();
    
    /**
     * @brief Validate file access
     * @param filename Filename to validate
     * @param for_write Whether this is for write access
     * @return true if access is allowed, false otherwise
     */
    bool validateFileAccess(const std::string& filename, bool for_write);
    
    /**
     * @brief Update activity timestamp
     */
    void updateActivity();
    
    /**
     * @brief Check if connection has timed out
     * @return true if timed out, false otherwise
     */
    bool hasTimedOut() const;
    
    /**
     * @brief Set connection state
     * @param new_state New connection state
     * @param message Optional message
     */
    void setState(TftpConnectionState new_state, const std::string& message = "");
    
    /**
     * @brief Log connection event
     * @param level Log level
     * @param message Message to log
     */
    void logEvent(LogLevel level, const std::string& message);
};

} // namespace simple_tftpd
