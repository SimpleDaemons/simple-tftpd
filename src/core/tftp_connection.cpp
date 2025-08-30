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
    // Basic implementation - just log for now
}

void TftpConnection::handleWriteRequest(const TftpRequestPacket& packet) {
    logEvent(LogLevel::INFO, "Handling write request for file: " + packet.getFilename());
    // Basic implementation - just log for now
}

void TftpConnection::handleDataPacket(const TftpDataPacket& packet) {
    logEvent(LogLevel::DEBUG, "Handling data packet with block " + std::to_string(packet.getBlockNumber()));
    // Basic implementation - just log for now
}

void TftpConnection::handleAckPacket(const TftpAckPacket& packet) {
    logEvent(LogLevel::DEBUG, "Handling ACK packet for block " + std::to_string(packet.getBlockNumber()));
    // Basic implementation - just log for now
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
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::sendAcknowledgment(uint16_t block_number) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::openReadFile(const std::string& filename) {
    // Basic implementation - just return true for now
    return true;
}

bool TftpConnection::openWriteFile(const std::string& filename) {
    // Basic implementation - just return true for now
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
    // Basic implementation - just return true for now
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

} // namespace simple_tftpd
