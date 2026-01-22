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

#include "simple-tftpd/core/tftp/connection.hpp"
#include "simple-tftpd/core/tftp/server.hpp"
#include "simple-tftpd/production/security/manager.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <limits>

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
      next_block_to_send_(1),
      last_ack_block_(0),
      max_retries_(config ? config->getMaxRetries() : 5),
      awaiting_data_(false),
      sent_option_ack_(false),
      awaiting_oack_ack_(false),
      final_block_sent_(false),
      final_block_number_(0),
      negotiated_block_size_(config ? config->getBlockSize() : 512),
      negotiated_window_size_(config ? config->getWindowSize() : 1),
      current_file_size_(0),
      advertised_file_size_(0),
      ack_retry_count_(0),
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

    in_flight_blocks_.clear();
    next_block_to_send_ = 1;
    last_ack_block_ = 0;
    final_block_sent_ = false;
    final_block_number_ = 0;
    awaiting_data_ = false;
    ack_retry_count_ = 0;

    // Process TFTP options
    TftpOptions options = packet.getOptions();
    if (!applyRequestOptions(options, true)) {
        return;
    }

    setState(TftpConnectionState::TRANSFERRING, "Starting file transfer");

    if (awaiting_oack_ack_) {
        // Wait for ACK(0) before streaming data
        return;
    }

    if (!fillSendWindow()) {
        sendError(TftpError::NETWORK_ERROR, "Failed to send data");
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
    if (options.has_tsize && config_ && options.tsize > config_->getMaxFileSize()) {
        sendError(TftpError::DISK_FULL, "Requested transfer size exceeds server limit");
        return;
    }

    if (!applyRequestOptions(options, false)) {
        return;
    }

    // Prepare to receive data
    current_block_ = 0;
    expected_block_ = 1;
    awaiting_data_ = true;
    ack_retry_count_ = 0;
    last_ack_block_ = 0;
    last_ack_time_ = std::chrono::steady_clock::now();
    current_file_size_ = 0;
    if (options.has_tsize) {
        advertised_file_size_ = options.tsize;
    } else {
        advertised_file_size_ = 0;
    }

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
        logEvent(LogLevel::DEBUG, "Duplicate DATA block " + std::to_string(block_number) + ", re-sending ACK");
        sendAcknowledgment(block_number, false);
        return;
    }

    if (block_number != expected_block_) {
        logEvent(LogLevel::WARNING, "Out of order block: " + std::to_string(block_number) +
                ", expected: " + std::to_string(expected_block_));
        sendAcknowledgment(current_block_, false);
        return;
    }

    // Process data based on transfer mode
    std::vector<uint8_t> processed_data = processDataForMode(data, transfer_mode_, false);

    current_file_size_ += processed_data.size();
    if (config_ && current_file_size_ > config_->getMaxFileSize()) {
        sendError(TftpError::DISK_FULL, "File exceeds configured size limit");
        return;
    }

    if (advertised_file_size_ > 0 && current_file_size_ > advertised_file_size_) {
        sendError(TftpError::DISK_FULL, "Client exceeded advertised transfer size");
        return;
    }

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

    if (processed_data.size() < negotiated_block_size_) {
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

    if (awaiting_oack_ack_) {
        if (block_number == 0) {
            awaiting_oack_ack_ = false;
            if (!fillSendWindow()) {
                sendError(TftpError::NETWORK_ERROR, "Failed to send data");
            }
        }
        return;
    }

    auto in_flight = in_flight_blocks_.find(block_number);
    if (in_flight == in_flight_blocks_.end()) {
        logEvent(LogLevel::DEBUG, "Duplicate ACK for block " + std::to_string(block_number));
        return;
    }

    bool was_final = in_flight->second.is_final;
    in_flight_blocks_.erase(in_flight);

    last_ack_block_ = block_number;
    current_block_ = block_number;

    if (was_final && in_flight_blocks_.empty()) {
        setState(TftpConnectionState::COMPLETED, "File transfer completed");
        closeFiles();
        active_.store(false);
        return;
    }

    if (!fillSendWindow() && in_flight_blocks_.empty() && final_block_sent_) {
        // No more data to send; wait for outstanding ACKs
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

    for (auto& entry : in_flight_blocks_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.second.last_sent);
        if (elapsed >= timeout_) {
            if (entry.second.retries >= max_retries_) {
                logEvent(LogLevel::ERROR, "Retry limit reached for block " + std::to_string(entry.first));
                sendError(TftpError::TIMEOUT, "Retry limit exceeded");
                active_.store(false);
                setState(TftpConnectionState::ERROR, "Retry limit exceeded");
                return false;
            }

            if (!resendBlock(entry.first)) {
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
    auto now = std::chrono::steady_clock::now();

    if (is_retry) {
        return resendBlock(block_number);
    }

    if (!read_file_.is_open()) {
        return false;
    }

    std::vector<uint8_t> buffer(negotiated_block_size_);
    read_file_.read(reinterpret_cast<char*>(buffer.data()), negotiated_block_size_);
    std::streamsize bytes_read = read_file_.gcount();

    if (bytes_read < 0) {
        logEvent(LogLevel::ERROR, "Failed to read from file");
        return false;
    }

    bool eof_block = static_cast<size_t>(bytes_read) < negotiated_block_size_;
    if (bytes_read == 0 && !read_file_.eof()) {
        logEvent(LogLevel::ERROR, "Unexpected zero-byte read");
        return false;
    }

    std::vector<uint8_t> payload;
    buffer.resize(static_cast<size_t>(bytes_read));
    payload = processDataForMode(buffer, transfer_mode_, true);

    InFlightBlock block;
    block.payload = payload;
    block.is_final = eof_block;
    block.last_sent = now;
    block.retries = 0;

    TftpDataPacket data_packet(block_number, block.payload);
    std::vector<uint8_t> packet_data = data_packet.serialize();

    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to send data packet");
        return false;
    }

    in_flight_blocks_[block_number] = std::move(block);
    bytes_transferred_ += payload.size();
    updateActivity();

    if (in_flight_blocks_[block_number].is_final) {
        final_block_sent_ = true;
        final_block_number_ = block_number;
    }

    next_block_to_send_ = block_number + 1;
    return true;
}

bool TftpConnection::fillSendWindow() {
    bool sent_any = false;
    while (in_flight_blocks_.size() < negotiated_window_size_) {
        if (final_block_sent_ && next_block_to_send_ > final_block_number_) {
            break;
        }

        if (!sendDataBlock(next_block_to_send_, false)) {
            return sent_any || !in_flight_blocks_.empty();
        }

        sent_any = true;

        if (final_block_sent_ && next_block_to_send_ > final_block_number_) {
            break;
        }
    }
    return sent_any || !in_flight_blocks_.empty();
}

bool TftpConnection::resendBlock(uint16_t block_number) {
    auto it = in_flight_blocks_.find(block_number);
    if (it == in_flight_blocks_.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    TftpDataPacket data_packet(block_number, it->second.payload);
    std::vector<uint8_t> packet_data = data_packet.serialize();

    if (!server_.sendPacket(packet_data.data(), packet_data.size(), client_addr_, client_port_)) {
        logEvent(LogLevel::ERROR, "Failed to resend data packet");
        return false;
    }

    it->second.last_sent = now;
    it->second.retries++;
    updateActivity();
    return true;
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

    awaiting_data_ = track_state;
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
    advertised_file_size_ = static_cast<uint64_t>(file_size);

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
    // Use production security manager if available
    if (security_manager_) {
        return security_manager_->validateFileAccess(filename, client_addr_, for_write);
    }

    // Fallback to basic validation if no security manager
    if (!config_) {
        return false;
    }

    // Basic operation checks
    if (for_write && !config_->isWriteEnabled()) {
        logEvent(LogLevel::WARNING, "Write operations are disabled");
        return false;
    }

    if (!for_write && !config_->isReadEnabled()) {
        logEvent(LogLevel::WARNING, "Read operations are disabled");
        return false;
    }

    // Basic filename validation
    const size_t TFTP_MAX_FILENAME_LENGTH = 512;
    if (filename.empty() || filename.length() > TFTP_MAX_FILENAME_LENGTH) {
        logEvent(LogLevel::WARNING, "Invalid filename: " + filename);
        return false;
    }

    // Basic path traversal check
    if (filename.find("..") != std::string::npos || filename.find("/") == 0) {
        logEvent(LogLevel::WARNING, "Path traversal attempt detected: " + filename);
        return false;
    }

    return true;
}

void TftpConnection::setSecurityManager(std::shared_ptr<ProductionSecurityManager> security_manager) {
    security_manager_ = security_manager;
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

    // Add options based on flags
    if (options.has_blksize) {
        appendOption("blksize", std::to_string(options.blksize));
    }

    if (options.has_timeout) {
        appendOption("timeout", std::to_string(options.timeout));
    }

    if (options.has_tsize) {
        appendOption("tsize", std::to_string(options.tsize));
    }

    if (options.has_windowsize) {
        appendOption("windowsize", std::to_string(options.windowsize));
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

bool TftpConnection::applyRequestOptions(const TftpOptions& request_options, bool is_read_request) {
    if (config_) {
        negotiated_block_size_ = config_->getBlockSize();
        negotiated_window_size_ = config_->getWindowSize();
        timeout_ = std::chrono::seconds(config_->getTimeout());
        max_retries_ = config_->getMaxRetries();
    }

    TftpOptions response;
    bool needs_oack = false;

    if (request_options.has_blksize) {
        uint16_t desired = std::clamp<uint16_t>(request_options.blksize, 8, 65464);
        if (config_) {
            desired = std::min<uint16_t>(desired, config_->getBlockSize());
        }
        negotiated_block_size_ = desired;
        response.has_blksize = true;
        response.blksize = desired;
        needs_oack = true;
    }

    if (request_options.has_timeout) {
        uint16_t desired = std::clamp<uint16_t>(request_options.timeout, 1, 255);
        timeout_ = std::chrono::seconds(desired);
        response.has_timeout = true;
        response.timeout = desired;
        needs_oack = true;
    }

    if (request_options.has_windowsize) {
        uint16_t upper = config_ ? std::max<uint16_t>(static_cast<uint16_t>(1), config_->getWindowSize()) : request_options.windowsize;
        uint16_t desired = std::clamp<uint16_t>(request_options.windowsize, 1, upper);
        negotiated_window_size_ = desired;
        response.has_windowsize = true;
        response.windowsize = desired;
        needs_oack = true;
    }

    if (request_options.has_tsize) {
        if (is_read_request) {
            response.has_tsize = true;
            response.tsize = static_cast<uint32_t>(std::min<uint64_t>(advertised_file_size_, std::numeric_limits<uint32_t>::max()));
        } else {
            advertised_file_size_ = request_options.tsize;
            current_file_size_ = 0;
            response.has_tsize = true;
            response.tsize = request_options.tsize;
        }
        needs_oack = true;
    } else if (is_read_request) {
        // Provide file size if requested implicitly
        response.has_tsize = true;
        response.tsize = static_cast<uint32_t>(std::min<uint64_t>(advertised_file_size_, std::numeric_limits<uint32_t>::max()));
    }

    if (!needs_oack) {
        sent_option_ack_ = false;
        awaiting_oack_ack_ = false;
        return true;
    }

    sent_option_ack_ = true;
    awaiting_oack_ack_ = is_read_request;
    return sendOptionAck(response);
}

} // namespace simple_tftpd
