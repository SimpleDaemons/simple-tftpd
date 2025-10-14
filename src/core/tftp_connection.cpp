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

#include "simple_tftpd/tftp_connection.hpp"
#include <iostream>
#include <filesystem>

namespace simple_tftpd {

TftpConnection::TftpConnection(TftpServer& server,
                             const std::string& client_addr,
                             port_t client_port,
                             std::shared_ptr<TftpConfig> config,
                             std::shared_ptr<Logger> logger)
    : server_(server),
      client_addr_(client_addr),
      client_port_(client_port),
      config_(config),
      logger_(logger),
      state_(TftpConnectionState::INITIALIZED),
      direction_(TftpTransferDirection::READ),
      current_block_(0),
      expected_block_(0),
      bytes_transferred_(0),
      timeout_(std::chrono::seconds(5)),
      active_(false) {
    
    start_time_ = std::chrono::steady_clock::now();
    last_activity_ = start_time_;
}

TftpConnection::~TftpConnection() {
    stop();
}

bool TftpConnection::start() {
    if (active_.load()) {
        return false;
    }
    
    active_.store(true);
    setState(TftpConnectionState::CONNECTED, "Connection started");
    
    // Start worker thread
    worker_thread_ = std::thread(&TftpConnection::workerThread, this);
    
    return true;
}

void TftpConnection::stop() {
    if (!active_.load()) {
        return;
    }
    
    active_.store(false);
    setState(TftpConnectionState::CLOSED, "Connection stopped");
    
    // Close files
    closeFiles();
    
    // Wait for worker thread to finish
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

bool TftpConnection::isActive() const {
    return active_.load();
}

TftpConnectionState TftpConnection::getState() const {
    return state_;
}

std::string TftpConnection::getClientAddress() const {
    return client_addr_;
}

port_t TftpConnection::getClientPort() const {
    return client_port_;
}

TftpTransferDirection TftpConnection::getTransferDirection() const {
    return direction_;
}

std::string TftpConnection::getFilename() const {
    return filename_;
}

size_t TftpConnection::getBytesTransferred() const {
    return bytes_transferred_;
}

std::chrono::steady_clock::time_point TftpConnection::getStartTime() const {
    return start_time_;
}

std::chrono::seconds TftpConnection::getDuration() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
}

void TftpConnection::handlePacket(const uint8_t* packet_data,
                                size_t packet_size,
                                const std::string& sender_addr,
                                port_t sender_port) {
    // Basic packet handling - just log for now
    logEvent(LogLevel::DEBUG, "Received packet of size " + std::to_string(packet_size));
    
    updateActivity();
}

bool TftpConnection::sendPacket(const TftpPacket& packet) {
    // Basic packet sending - just log for now
    logEvent(LogLevel::DEBUG, "Sending " + packet.getTypeString() + " packet");
    return true;
}

bool TftpConnection::sendError(TftpError error_code, const std::string& error_message) {
    TftpErrorPacket error_packet(error_code, error_message);
    return sendPacket(error_packet);
}

void TftpConnection::setTimeout(std::chrono::seconds timeout) {
    timeout_ = timeout;
}

std::chrono::seconds TftpConnection::getTimeout() const {
    return timeout_;
}

void TftpConnection::setCallback(std::function<void(TftpConnectionState, const std::string&)> callback) {
    callback_ = callback;
}

void TftpConnection::workerThread() {
    while (active_.load()) {
        // Basic worker thread - just sleep for now
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Check for timeout
        if (hasTimedOut()) {
            logEvent(LogLevel::WARNING, "Connection timed out");
            break;
        }
    }
    
    setState(TftpConnectionState::CLOSED, "Worker thread finished");
}

void TftpConnection::handleReadRequest(const TftpRequestPacket& packet) {
    logEvent(LogLevel::INFO, "Handling read request for file: " + packet.getFilename());
    
    // Set transfer direction and mode
    direction_ = TftpTransferDirection::READ;
    transfer_mode_ = packet.getMode();
    
    // Validate file access
    if (!validateFileAccess(packet.getFilename(), false)) {
        sendError(TftpError::ACCESS_VIOLATION, "Access denied");
        return;
    }
    
    // Open file for reading
    if (!openReadFile(packet.getFilename())) {
        sendError(TftpError::FILE_NOT_FOUND, "File not found");
        return;
    }
    
    // Process TFTP options
    TftpOptions options = packet.getOptions();
    if (options.blksize != 512) {
        // Send OACK for blksize option
        sendOptionAck(options);
        return;
    }
    
    // Start sending data
    current_block_ = 1;
    setState(TftpConnectionState::SENDING_DATA, "Starting file transfer");
    
    if (!sendDataBlock(current_block_)) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send data");
        return;
    }
}

void TftpConnection::handleWriteRequest(const TftpRequestPacket& packet) {
    logEvent(LogLevel::INFO, "Handling write request for file: " + packet.getFilename());
    
    // Set transfer direction and mode
    direction_ = TftpTransferDirection::WRITE;
    transfer_mode_ = packet.getMode();
    
    // Validate file access
    if (!validateFileAccess(packet.getFilename(), true)) {
        sendError(TftpError::ACCESS_VIOLATION, "Access denied");
        return;
    }
    
    // Open file for writing
    if (!openWriteFile(packet.getFilename())) {
        sendError(TftpError::FILE_EXISTS, "File already exists or cannot be created");
        return;
    }
    
    // Process TFTP options
    TftpOptions options = packet.getOptions();
    if (options.blksize != 512) {
        // Send OACK for blksize option
        sendOptionAck(options);
        return;
    }
    
    // Send ACK for write request
    current_block_ = 0;
    setState(TftpConnectionState::RECEIVING_DATA, "Ready to receive file");
    
    if (!sendAcknowledgment(current_block_)) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send ACK");
        return;
    }
}

void TftpConnection::handleDataPacket(const TftpDataPacket& packet) {
    logEvent(LogLevel::DEBUG, "Handling data packet with block " + std::to_string(packet.getBlockNumber()));
    
    if (direction_ != TftpTransferDirection::WRITE) {
        sendError(TftpError::ILLEGAL_OPERATION, "Unexpected data packet");
        return;
    }
    
    uint16_t block_number = packet.getBlockNumber();
    const std::vector<uint8_t>& data = packet.getFileData();
    
    // Check if this is the expected block
    if (block_number != current_block_ + 1) {
        logEvent(LogLevel::WARNING, "Unexpected block number: " + std::to_string(block_number) + ", expected: " + std::to_string(current_block_ + 1));
        return;
    }
    
    // Process data based on transfer mode
    std::vector<uint8_t> processed_data = processDataForMode(data, transfer_mode_, false);
    
    // Write data to file
    if (write_file_.is_open()) {
        write_file_.write(reinterpret_cast<const char*>(processed_data.data()), processed_data.size());
        if (write_file_.fail()) {
            sendError(TftpError::DISK_FULL, "Failed to write data");
            return;
        }
    }
    
    // Update counters
    current_block_ = block_number;
    bytes_transferred_ += processed_data.size();
    
    // Send ACK
    if (!sendAcknowledgment(block_number)) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send ACK");
        return;
    }
    
    // Check if this is the last block (less than full block size)
    if (data.size() < config_->getBlockSize()) {
        setState(TftpConnectionState::COMPLETED, "File transfer completed");
        closeFiles();
    }
}

void TftpConnection::handleAckPacket(const TftpAckPacket& packet) {
    logEvent(LogLevel::DEBUG, "Handling ACK packet for block " + std::to_string(packet.getBlockNumber()));
    
    if (direction_ != TftpTransferDirection::READ) {
        sendError(TftpError::ILLEGAL_OPERATION, "Unexpected ACK packet");
        return;
    }
    
    uint16_t block_number = packet.getBlockNumber();
    
    // Check if this is the expected ACK
    if (block_number != current_block_) {
        logEvent(LogLevel::WARNING, "Unexpected ACK block number: " + std::to_string(block_number) + ", expected: " + std::to_string(current_block_));
        return;
    }
    
    // Send next data block
    current_block_++;
    if (!sendDataBlock(current_block_)) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send data");
        return;
    }
}

void TftpConnection::handleErrorPacket(const TftpErrorPacket& packet) {
    logEvent(LogLevel::ERROR, "Handling error packet: " + packet.getErrorMessage());
    // Basic implementation - just log for now
}

bool TftpConnection::processReadRequest(const TftpRequestPacket& packet) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::processWriteRequest(const TftpRequestPacket& packet) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::sendDataBlock(uint16_t block_number) {
    if (!read_file_.is_open()) {
        return false;
    }
    
    // Read data from file
    std::vector<uint8_t> buffer(config_->getBlockSize());
    read_file_.read(reinterpret_cast<char*>(buffer.data()), config_->getBlockSize());
    std::streamsize bytes_read = read_file_.gcount();
    
    if (bytes_read < 0) {
        logEvent(LogLevel::ERROR, "Failed to read from file");
        return false;
    }
    
    // Resize buffer to actual bytes read
    buffer.resize(static_cast<size_t>(bytes_read));
    
    // Process data based on transfer mode
    std::vector<uint8_t> processed_data = processDataForMode(buffer, transfer_mode_, true);
    
    // Create and send data packet
    TftpDataPacket data_packet(block_number, processed_data);
    std::vector<uint8_t> packet_data = data_packet.serialize();
    
    // Send packet via server
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send data packet");
        return false;
    }
    
    bytes_transferred_ += processed_data.size();
    updateActivity();
    
    // Check if this is the last block
    if (bytes_read < static_cast<std::streamsize>(config_->getBlockSize())) {
        setState(TftpConnectionState::COMPLETED, "File transfer completed");
        closeFiles();
    }
    
    return true;
}

bool TftpConnection::sendAcknowledgment(uint16_t block_number) {
    // Create and send ACK packet
    TftpAckPacket ack_packet(block_number);
    std::vector<uint8_t> packet_data = ack_packet.serialize();
    
    // Send packet via server
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send ACK packet");
        return false;
    }
    
    updateActivity();
    return true;
}

bool TftpConnection::openReadFile(const std::string& filename) {
    if (!validateFileAccess(filename, false)) {
        return false;
    }
    
    // Build full path
    std::string full_path = config_->getRootDirectory() + "/" + filename;
    
    // Check if file exists
    std::ifstream test_file(full_path);
    if (!test_file.good()) {
        logEvent(LogLevel::WARNING, "File not found: " + full_path);
        return false;
    }
    test_file.close();
    
    // Check file size
    std::ifstream file(full_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        logEvent(LogLevel::ERROR, "Failed to open file for reading: " + full_path);
        return false;
    }
    
    std::streampos file_size = file.tellg();
    if (file_size > static_cast<std::streampos>(config_->getMaxFileSize())) {
        logEvent(LogLevel::WARNING, "File too large: " + std::to_string(file_size) + " bytes");
        file.close();
        return false;
    }
    
    file.close();
    
    // Open file for reading
    read_file_.open(full_path, std::ios::binary);
    if (!read_file_.is_open()) {
        logEvent(LogLevel::ERROR, "Failed to open file for reading: " + full_path);
        return false;
    }
    
    logEvent(LogLevel::INFO, "Opened file for reading: " + full_path);
    return true;
}

bool TftpConnection::openWriteFile(const std::string& filename) {
    if (!validateFileAccess(filename, true)) {
        return false;
    }
    
    // Build full path
    std::string full_path = config_->getRootDirectory() + "/" + filename;
    
    // Check if file already exists and overwrite protection is enabled
    if (config_->isOverwriteProtectionEnabled()) {
        std::ifstream test_file(full_path);
        if (test_file.good()) {
            logEvent(LogLevel::WARNING, "File already exists and overwrite protection is enabled: " + full_path);
            test_file.close();
            return false;
        }
        test_file.close();
    }
    
    // Create directory if it doesn't exist
    std::string dir_path = full_path.substr(0, full_path.find_last_of('/'));
    if (!dir_path.empty()) {
        std::filesystem::create_directories(dir_path);
    }
    
    // Open file for writing
    write_file_.open(full_path, std::ios::binary | std::ios::out);
    if (!write_file_.is_open()) {
        logEvent(LogLevel::ERROR, "Failed to open file for writing: " + full_path);
        return false;
    }
    
    logEvent(LogLevel::INFO, "Opened file for writing: " + full_path);
    return true;
}

void TftpConnection::closeFiles() {
    if (read_file_.is_open()) {
        read_file_.close();
    }
    if (write_file_.is_open()) {
        write_file_.close();
    }
}

bool TftpConnection::validateFileAccess(const std::string& filename, bool for_write) {
    if (!config_) {
        return false;
    }
    
    // Check if operation is allowed
    if (for_write && !config_->isWriteEnabled()) {
        logEvent(LogLevel::WARNING, "Write operations are disabled");
        return false;
    }
    
    if (!for_write && !config_->isReadEnabled()) {
        logEvent(LogLevel::WARNING, "Read operations are disabled");
        return false;
    }
    
    // Validate filename
    if (filename.empty() || filename.length() > TFTP_MAX_FILENAME_LENGTH) {
        logEvent(LogLevel::WARNING, "Invalid filename: " + filename);
        return false;
    }
    
    // Check for path traversal attacks
    if (filename.find("..") != std::string::npos || filename.find("/") == 0) {
        logEvent(LogLevel::WARNING, "Path traversal attempt detected: " + filename);
        return false;
    }
    
    // Build full path
    std::string full_path = config_->getRootDirectory() + "/" + filename;
    
    // Normalize path (remove double slashes, etc.)
    std::string normalized_path;
    bool last_was_slash = false;
    for (char c : full_path) {
        if (c == '/') {
            if (!last_was_slash) {
                normalized_path += c;
            }
            last_was_slash = true;
        } else {
            normalized_path += c;
            last_was_slash = false;
        }
    }
    
    // Ensure path is within root directory
    if (normalized_path.find(config_->getRootDirectory()) != 0) {
        logEvent(LogLevel::WARNING, "Path outside root directory: " + normalized_path);
        return false;
    }
    
    // Check if directory is allowed
    std::string dir_path = normalized_path.substr(0, normalized_path.find_last_of('/'));
    if (!config_->isDirectoryAllowed(dir_path)) {
        logEvent(LogLevel::WARNING, "Directory not allowed: " + dir_path);
        return false;
    }
    
    return true;
}

void TftpConnection::updateActivity() {
    last_activity_ = std::chrono::steady_clock::now();
}

bool TftpConnection::hasTimedOut() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_activity_);
    return elapsed > timeout_;
}

void TftpConnection::setState(TftpConnectionState new_state, const std::string& message) {
    state_ = new_state;
    
    if (callback_) {
        callback_(new_state, message);
    }
    
    logEvent(LogLevel::INFO, "State changed to " + std::to_string(static_cast<int>(new_state)) + ": " + message);
}

void TftpConnection::logEvent(LogLevel level, const std::string& message) {
    if (logger_) {
        logger_->log(level, "[Connection " + client_addr_ + ":" + std::to_string(client_port_) + "] " + message);
    }
}

std::vector<uint8_t> TftpConnection::processDataForMode(const std::vector<uint8_t>& data, TftpMode mode, bool is_sending) {
    std::vector<uint8_t> result = data;
    
    switch (mode) {
        case TftpMode::NETASCII:
            if (is_sending) {
                // Convert LF to CRLF for netascii mode
                result.clear();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] == '\n' && (i == 0 || data[i-1] != '\r')) {
                        result.push_back('\r');
                    }
                    result.push_back(data[i]);
                }
            } else {
                // Convert CRLF to LF for netascii mode
                result.clear();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] == '\r' && i + 1 < data.size() && data[i+1] == '\n') {
                        result.push_back('\n');
                        ++i; // Skip the LF
                    } else if (data[i] != '\r') {
                        result.push_back(data[i]);
                    }
                }
            }
            break;
            
        case TftpMode::OCTET:
            // No conversion needed for binary mode
            break;
            
        case TftpMode::MAIL:
            // Mail mode is similar to netascii but with additional processing
            // For now, treat it the same as netascii
            if (is_sending) {
                result.clear();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] == '\n' && (i == 0 || data[i-1] != '\r')) {
                        result.push_back('\r');
                    }
                    result.push_back(data[i]);
                }
            } else {
                result.clear();
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] == '\r' && i + 1 < data.size() && data[i+1] == '\n') {
                        result.push_back('\n');
                        ++i; // Skip the LF
                    } else if (data[i] != '\r') {
                        result.push_back(data[i]);
                    }
                }
            }
            break;
    }
    
    return result;
}

bool TftpConnection::sendError(TftpError error_code, const std::string& error_message) {
    // Create and send error packet
    TftpErrorPacket error_packet(error_code, error_message);
    std::vector<uint8_t> packet_data = error_packet.serialize();
    
    // Send packet via server
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send error packet");
        return false;
    }
    
    setState(TftpConnectionState::ERROR, error_message);
    updateActivity();
    return true;
}

bool TftpConnection::sendOptionAck(const TftpOptions& options) {
    // Create OACK packet (Option Acknowledgment)
    std::vector<uint8_t> packet_data;
    
    // Add opcode (OACK = 6)
    packet_data.push_back(0);
    packet_data.push_back(6);
    
    // Add options
    if (options.blksize != 512) {
        std::string blksize_str = std::to_string(options.blksize);
        packet_data.insert(packet_data.end(), "blksize", "blksize" + 7);
        packet_data.push_back(0);
        packet_data.insert(packet_data.end(), blksize_str.begin(), blksize_str.end());
        packet_data.push_back(0);
    }
    
    if (options.timeout != 5) {
        std::string timeout_str = std::to_string(options.timeout);
        packet_data.insert(packet_data.end(), "timeout", "timeout" + 7);
        packet_data.push_back(0);
        packet_data.insert(packet_data.end(), timeout_str.begin(), timeout_str.end());
        packet_data.push_back(0);
    }
    
    if (options.tsize != 0) {
        std::string tsize_str = std::to_string(options.tsize);
        packet_data.insert(packet_data.end(), "tsize", "tsize" + 5);
        packet_data.push_back(0);
        packet_data.insert(packet_data.end(), tsize_str.begin(), tsize_str.end());
        packet_data.push_back(0);
    }
    
    // Send packet via server
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send OACK packet");
        return false;
    }
    
    updateActivity();
    return true;
}

} // namespace simple_tftpd
