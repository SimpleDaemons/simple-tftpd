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
#include "simple_tftpd/tftp_server.hpp"
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
      bytes_transferred_(0),
      current_block_(0),
      expected_block_(0),
      start_time_(std::chrono::steady_clock::now()),
      last_activity_(start_time_),
      timeout_(std::chrono::seconds(config ? config->getTimeout() : 5)),
      active_(false),
      last_block_sent_(0),
      last_ack_block_(0),
      awaiting_ack_(false),
      awaiting_data_(false),
      last_block_final_(false),
      retry_count_(0),
      ack_retry_count_(0),
      max_retries_(config ? config->getMaxRetries() : 5),
      last_packet_time_(start_time_),
      last_ack_time_(start_time_) {}

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
    bool was_active = active_.exchange(false);
    
    if (was_active &&
        state_ != TftpConnectionState::COMPLETED &&
        state_ != TftpConnectionState::ERROR) {
        setState(TftpConnectionState::CLOSED, "Connection stopped");
    }
    
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
    std::vector<uint8_t> packet_data = packet.serialize();
    
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send " + packet.getTypeString() + " packet");
        return false;
    }
    
    updateActivity();
    return true;
}

bool TftpConnection::sendError(TftpError error_code, const std::string& error_message) {
    TftpErrorPacket error_packet(error_code, error_message);
    bool sent = sendPacket(error_packet);
    setState(TftpConnectionState::ERROR, error_message);
    active_.store(false);
    closeFiles();
    return sent;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!handleTimeoutTick()) {
            break;
        }
    }
    
    if (state_ != TftpConnectionState::COMPLETED &&
        state_ != TftpConnectionState::ERROR &&
        state_ != TftpConnectionState::CLOSED) {
        setState(TftpConnectionState::CLOSED, "Worker thread finished");
    }
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
    
    // Initialize transfer state
    current_block_ = 0;
    awaiting_ack_ = false;
    last_block_final_ = false;
    retry_count_ = 0;
    last_data_block_.clear();
    setState(TftpConnectionState::TRANSFERRING, "Starting file transfer");
    
    if (!sendNextDataBlock()) {
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
    
    // Prepare to receive data
    current_block_ = 0;
    expected_block_ = 1;
    awaiting_data_ = true;
    ack_retry_count_ = 0;
    last_ack_block_ = 0;
    last_ack_time_ = std::chrono::steady_clock::now();
    setState(TftpConnectionState::TRANSFERRING, "Ready to receive file");
    
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
    
    if (block_number <= current_block_) {
        // Duplicate packet, re-acknowledge last block
        logEvent(LogLevel::DEBUG, "Duplicate DATA block " + std::to_string(block_number) + ", re-sending ACK");
        sendAcknowledgment(block_number, false);
        return;
    }
    
    // Check if this is the expected block
    if (block_number != expected_block_) {
        logEvent(LogLevel::WARNING, "Out of order block: " + std::to_string(block_number) +
                ", expected: " + std::to_string(expected_block_));
        sendAcknowledgment(current_block_, false);
        return;
    }
    
    // Process data based on transfer mode
    std::vector<uint8_t> processed_data = processDataForMode(data, transfer_mode_, false);
    
    if (config_ && (bytes_transferred_ + processed_data.size()) > config_->getMaxFileSize()) {
        sendError(TftpError::DISK_FULL, "Maximum file size exceeded");
        return;
    }
    
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
    expected_block_ = block_number + 1;
    bytes_transferred_ += processed_data.size();
    
    // Send ACK
    if (!sendAcknowledgment(block_number)) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send ACK");
        return;
    }
    
    // Check if this is the last block (less than full block size)
    if (data.size() < config_->getBlockSize()) {
        awaiting_data_ = false;
        setState(TftpConnectionState::COMPLETED, "File transfer completed");
        closeFiles();
        active_.store(false);
    }
}

void TftpConnection::handleAckPacket(const TftpAckPacket& packet) {
    logEvent(LogLevel::DEBUG, "Handling ACK packet for block " + std::to_string(packet.getBlockNumber()));
    
    if (direction_ != TftpTransferDirection::READ) {
        sendError(TftpError::ILLEGAL_OPERATION, "Unexpected ACK packet");
        return;
    }
    
    uint16_t block_number = packet.getBlockNumber();
    
    if (block_number < current_block_) {
        // Duplicate ACK for already processed block
        logEvent(LogLevel::DEBUG, "Duplicate ACK for block " + std::to_string(block_number));
        return;
    }
    
    if (block_number != last_block_sent_) {
        logEvent(LogLevel::WARNING, "Unexpected ACK block number: " + std::to_string(block_number) +
                ", expected: " + std::to_string(last_block_sent_));
        return;
    }
    
    awaiting_ack_ = false;
    retry_count_ = 0;
    current_block_ = block_number;
    last_data_block_.clear();
    
    if (last_block_final_ && block_number == last_block_sent_) {
        setState(TftpConnectionState::COMPLETED, "File transfer completed");
        closeFiles();
        active_.store(false);
        return;
    }
    
    if (!sendNextDataBlock()) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send data");
    }
}

void TftpConnection::handleErrorPacket(const TftpErrorPacket& packet) {
    logEvent(LogLevel::ERROR, "Handling error packet: " + packet.getErrorMessage());
    
    // Set connection state to error
    setState(TftpConnectionState::ERROR, packet.getErrorMessage());
    
    // Close files and cleanup
    closeFiles();
    
    // Stop the connection
    stop();
}

bool TftpConnection::processReadRequest(const TftpRequestPacket& packet) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::processWriteRequest(const TftpRequestPacket& packet) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::handleTimeoutTick() {
    auto now = std::chrono::steady_clock::now();
    
    if (hasTimedOut()) {
        logEvent(LogLevel::WARNING, "Connection idle timeout reached");
        handleTimeout();
        return false;
    }
    
    if (awaiting_ack_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_packet_time_);
        if (elapsed >= timeout_) {
            if (retry_count_ >= max_retries_) {
                logEvent(LogLevel::ERROR, "Retry limit reached while waiting for ACK");
                sendError(TftpError::TIMEOUT, "Retry limit exceeded");
                active_.store(false);
                setState(TftpConnectionState::ERROR, "Retry limit exceeded");
                return false;
            }
            
            ++retry_count_;
            if (!resendLastDataBlock()) {
                return false;
            }
        }
    }
    
    if (direction_ == TftpTransferDirection::WRITE && awaiting_data_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_ack_time_);
        if (elapsed >= timeout_) {
            if (ack_retry_count_ >= max_retries_) {
                logEvent(LogLevel::ERROR, "Retry limit reached while waiting for DATA");
                sendError(TftpError::TIMEOUT, "Client did not continue transfer");
                active_.store(false);
                setState(TftpConnectionState::ERROR, "Client stalled");
                return false;
            }
            
            ++ack_retry_count_;
            logEvent(LogLevel::WARNING, "Resending ACK for block " + std::to_string(last_ack_block_));
            if (!sendAcknowledgment(last_ack_block_, false)) {
                return false;
            }
        }
    }
    
    return true;
}

bool TftpConnection::sendDataBlock(uint16_t block_number, bool is_retry) {
    if (!config_) {
        return false;
    }
    
    std::vector<uint8_t> payload;
    bool is_final_block = false;
    
    if (is_retry) {
        if (last_data_block_.empty() && !last_block_final_) {
            logEvent(LogLevel::ERROR, "No previous block available for retry");
            return false;
        }
        payload = last_data_block_;
        is_final_block = last_block_final_;
    } else {
        if (!read_file_.is_open()) {
            return false;
        }
        
        std::vector<uint8_t> buffer(config_->getBlockSize());
        read_file_.read(reinterpret_cast<char*>(buffer.data()), config_->getBlockSize());
        std::streamsize bytes_read = read_file_.gcount();
        
        if (bytes_read < 0) {
            logEvent(LogLevel::ERROR, "Failed to read from file");
            return false;
        }
        
        buffer.resize(static_cast<size_t>(bytes_read));
        payload = processDataForMode(buffer, transfer_mode_, true);
        last_data_block_ = payload;
        last_block_final_ = bytes_read < static_cast<std::streamsize>(config_->getBlockSize());
        last_block_sent_ = block_number;
        retry_count_ = 0;
        is_final_block = last_block_final_;
        bytes_transferred_ += payload.size();
    }
    
    TftpDataPacket data_packet(block_number, payload);
    std::vector<uint8_t> packet_data = data_packet.serialize();
    
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send data packet");
        return false;
    }
    
    awaiting_ack_ = true;
    last_packet_time_ = std::chrono::steady_clock::now();
    updateActivity();
    
    if (!is_retry) {
        last_block_sent_ = block_number;
        if (is_final_block) {
            // Wait for final ACK to mark completion
            logEvent(LogLevel::DEBUG, "Final block " + std::to_string(block_number) + " sent, awaiting ACK");
        }
    }
    
    return true;
}

bool TftpConnection::sendNextDataBlock() {
    return sendDataBlock(static_cast<uint16_t>(current_block_ + 1), false);
}

bool TftpConnection::resendLastDataBlock() {
    if (last_block_sent_ == 0) {
        return false;
    }
    logEvent(LogLevel::WARNING, "Retrying DATA block " + std::to_string(last_block_sent_) +
            " (attempt " + std::to_string(retry_count_) + "/" + std::to_string(max_retries_) + ")");
    return sendDataBlock(last_block_sent_, true);
}

bool TftpConnection::sendAcknowledgment(uint16_t block_number, bool track_state) {
    TftpAckPacket ack_packet(block_number);
    std::vector<uint8_t> packet_data = ack_packet.serialize();
    
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send ACK packet");
        return false;
    }
    
    if (track_state) {
        last_ack_block_ = block_number;
        ack_retry_count_ = 0;
    }
    
    awaiting_data_ = true;
    last_ack_time_ = std::chrono::steady_clock::now();
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
    
    // Enforce allowed file extensions (if configured)
    std::string::size_type dot_pos = filename.find_last_of('.');
    std::string extension = (dot_pos == std::string::npos) ? std::string() : filename.substr(dot_pos + 1);
    if (!config_->isExtensionAllowed(extension)) {
        logEvent(LogLevel::WARNING, "File extension not allowed: " + filename);
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

bool TftpConnection::sendOptionAck(const TftpOptions& options) {
    // Create OACK packet (Option Acknowledgment)
    std::vector<uint8_t> packet_data;
    
    // Add opcode (OACK = 6)
    packet_data.push_back(0);
    packet_data.push_back(6);
    
    auto appendString = [&packet_data](const std::string& value) {
        packet_data.insert(packet_data.end(), value.begin(), value.end());
    };
    
    auto appendOption = [&](const std::string& key, const std::string& value) {
        appendString(key);
        packet_data.push_back(0);
        appendString(value);
        packet_data.push_back(0);
    };
    
    // Add options
    if (options.blksize != 512) {
        std::string blksize_str = std::to_string(options.blksize);
        appendOption("blksize", blksize_str);
    }
    
    if (options.timeout != 5) {
        std::string timeout_str = std::to_string(options.timeout);
        appendOption("timeout", timeout_str);
    }
    
    if (options.tsize != 0) {
        std::string tsize_str = std::to_string(options.tsize);
        appendOption("tsize", tsize_str);
    }
    
    // Send packet via server
    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send OACK packet");
        return false;
    }
    
    updateActivity();
    return true;
}

bool TftpConnection::handleTimeout() {
    logEvent(LogLevel::WARNING, "Connection timeout");
    sendError(TftpError::TIMEOUT, "Connection timeout");
    setState(TftpConnectionState::ERROR, "Connection timeout");
    active_.store(false);
    closeFiles();
    return false;
}

bool TftpConnection::handleInvalidPacket(const std::string& reason) {
    logEvent(LogLevel::WARNING, "Invalid packet received: " + reason);
    sendError(TftpError::INVALID_PACKET, "Invalid packet: " + reason);
    return false;
}

bool TftpConnection::handleFileError(const std::string& operation, const std::string& filename) {
    logEvent(LogLevel::ERROR, "File " + operation + " error for: " + filename);
    
    TftpError error_code = TftpError::PLATFORM_ERROR;
    std::string error_message = "File " + operation + " error";
    
    if (operation == "read") {
        error_code = TftpError::FILE_NOT_FOUND;
        error_message = "File not found: " + filename;
    } else if (operation == "write") {
        error_code = TftpError::DISK_FULL;
        error_message = "Disk full or write error: " + filename;
    }
    
    sendError(error_code, error_message);
    return false;
}

} // namespace simple_tftpd
