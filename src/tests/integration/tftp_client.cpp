/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "tftp_client.hpp"
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <algorithm>
#include <map>

#ifdef PLATFORM_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

namespace simple_tftpd {
namespace test {

// Helper function to convert string mode to TftpMode enum
static TftpMode stringToTftpMode(const std::string& mode) {
    std::string lower_mode = mode;
    std::transform(lower_mode.begin(), lower_mode.end(), lower_mode.begin(), ::tolower);
    
    if (lower_mode == "netascii") {
        return TftpMode::NETASCII;
    } else if (lower_mode == "octet") {
        return TftpMode::OCTET;
    } else if (lower_mode == "mail") {
        return TftpMode::MAIL;
    }
    // Default to octet
    return TftpMode::OCTET;
}

TftpClient::TftpClient(const std::string& server_addr, port_t server_port)
    : server_addr_(server_addr),
      server_port_(server_port),
      client_socket_(INVALID_SOCKET_VALUE),
      client_port_(0),
      last_success_(false),
      timeout_(std::chrono::seconds(5)),
      block_size_(512),
      window_size_(1),
      transfer_timeout_(std::chrono::seconds(5)) {
    
#ifdef PLATFORM_WINDOWS
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    if (!initializeSocket()) {
        last_error_ = "Failed to initialize socket";
    }
}

TftpClient::~TftpClient() {
    closeSocket();
#ifdef PLATFORM_WINDOWS
    WSACleanup();
#endif
}

bool TftpClient::initializeSocket() {
    client_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket_ == INVALID_SOCKET_VALUE) {
        return false;
    }
    
    // Bind to any available port
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0; // Let OS choose port
    
    if (bind(client_socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket();
        return false;
    }
    
    // Get the assigned port
    socklen_t len = sizeof(addr);
    if (getsockname(client_socket_, reinterpret_cast<struct sockaddr*>(&addr), &len) == 0) {
        client_port_ = ntohs(addr.sin_port);
    }
    
    // Set non-blocking mode
#ifdef PLATFORM_WINDOWS
    u_long mode = 1;
    ioctlsocket(client_socket_, FIONBIO, &mode);
#else
    int flags = fcntl(client_socket_, F_GETFL, 0);
    fcntl(client_socket_, F_SETFL, flags | O_NONBLOCK);
#endif
    
    return true;
}

void TftpClient::closeSocket() {
    if (client_socket_ != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(client_socket_);
        client_socket_ = INVALID_SOCKET_VALUE;
    }
}

bool TftpClient::sendPacket(const std::vector<uint8_t>& packet) {
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    
    if (inet_pton(AF_INET, server_addr_.c_str(), &server_addr.sin_addr) != 1) {
        last_error_ = "Invalid server address";
        return false;
    }
    
    ssize_t sent = sendto(client_socket_,
                         reinterpret_cast<const char*>(packet.data()),
                         packet.size(),
                         0,
                         reinterpret_cast<struct sockaddr*>(&server_addr),
                         sizeof(server_addr));
    
    return sent == static_cast<ssize_t>(packet.size());
}

std::vector<uint8_t> TftpClient::receivePacket(std::chrono::milliseconds timeout) {
    std::vector<uint8_t> buffer(65536);
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed >= timeout) {
            return {}; // Timeout
        }
        
        ssize_t received = recvfrom(client_socket_,
                                   reinterpret_cast<char*>(buffer.data()),
                                   buffer.size(),
                                   0,
                                   reinterpret_cast<struct sockaddr*>(&from_addr),
                                   &from_len);
        
        if (received > 0) {
            buffer.resize(received);
            return buffer;
        }
        
        // Check if it's a real error or just no data
#ifdef PLATFORM_WINDOWS
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
            return {};
        }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            return {};
        }
#endif
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool TftpClient::handleOack(const std::vector<uint8_t>& packet, TftpOptions& negotiated_options) {
    if (packet.size() < 2) return false;
    
    uint16_t opcode = (packet[0] << 8) | packet[1];
    if (opcode != 6) return false; // OACK
    
    // Manually parse OACK options (same format as RRQ/WRQ options)
    negotiated_options = TftpOptions{};
    size_t offset = 2; // Skip opcode
    
    while (offset < packet.size()) {
        // Parse option name (null-terminated string)
        std::string option_name;
        while (offset < packet.size() && packet[offset] != 0) {
            option_name += static_cast<char>(packet[offset]);
            offset++;
        }
        
        if (offset >= packet.size()) break;
        offset++; // Skip null terminator
        
        // Parse option value (null-terminated string)
        std::string option_value;
        while (offset < packet.size() && packet[offset] != 0) {
            option_value += static_cast<char>(packet[offset]);
            offset++;
        }
        
        if (offset >= packet.size()) break;
        offset++; // Skip null terminator
        
        // Process known options
        if (option_name == "blksize") {
            try {
                negotiated_options.has_blksize = true;
                negotiated_options.blksize = static_cast<uint16_t>(std::stoul(option_value));
                block_size_ = negotiated_options.blksize;
            } catch (...) {
                // Invalid value, ignore
            }
        } else if (option_name == "timeout") {
            try {
                negotiated_options.has_timeout = true;
                negotiated_options.timeout = static_cast<uint16_t>(std::stoul(option_value));
                transfer_timeout_ = std::chrono::seconds(negotiated_options.timeout);
            } catch (...) {
                // Invalid value, ignore
            }
        } else if (option_name == "tsize") {
            try {
                negotiated_options.has_tsize = true;
                negotiated_options.tsize = static_cast<uint32_t>(std::stoul(option_value));
            } catch (...) {
                // Invalid value, ignore
            }
        } else if (option_name == "windowsize") {
            try {
                negotiated_options.has_windowsize = true;
                negotiated_options.windowsize = static_cast<uint16_t>(std::stoul(option_value));
                window_size_ = negotiated_options.windowsize;
            } catch (...) {
                // Invalid value, ignore
            }
        }
    }
    
    return true;
}

bool TftpClient::handleError(const std::vector<uint8_t>& packet, uint16_t& error_code, std::string& error_msg) {
    if (packet.size() < 4) return false;
    
    uint16_t opcode = (packet[0] << 8) | packet[1];
    if (opcode != 5) return false; // ERROR
    
    error_code = (packet[2] << 8) | packet[3];
    
    // Extract error message (null-terminated string starting at offset 4)
    if (packet.size() > 4) {
        size_t msg_start = 4;
        size_t msg_end = msg_start;
        while (msg_end < packet.size() && packet[msg_end] != 0) {
            msg_end++;
        }
        error_msg = std::string(reinterpret_cast<const char*>(packet.data() + msg_start), 
                               msg_end - msg_start);
    }
    
    return true;
}

void TftpClient::setTimeout(std::chrono::seconds timeout) {
    timeout_ = timeout;
}

std::vector<uint8_t> TftpClient::readFile(const std::string& filename,
                                          const std::string& mode,
                                          const TftpOptions& options) {
    last_success_ = false;
    last_error_.clear();
    
    // Reset negotiated options
    block_size_ = 512;
    window_size_ = 1;
    transfer_timeout_ = timeout_;
    
    // Create RRQ packet
    TftpMode tftp_mode = stringToTftpMode(mode);
    TftpRequestPacket rrq(TftpOpcode::RRQ, filename, tftp_mode);
    rrq.setOptions(options);
    
    std::vector<uint8_t> packet_data = rrq.serialize();
    if (!sendPacket(packet_data)) {
        last_error_ = "Failed to send RRQ";
        return {};
    }
    
    // Receive response (OACK or first DATA packet)
    std::vector<uint8_t> response = receivePacket(std::chrono::milliseconds(timeout_ * 1000));
    if (response.empty()) {
        last_error_ = "Timeout waiting for response";
        return {};
    }
    
    uint16_t opcode = (response[0] << 8) | response[1];
    
    // Handle OACK
    TftpOptions negotiated_options;
    if (opcode == 6) { // OACK
        if (!handleOack(response, negotiated_options)) {
            last_error_ = "Invalid OACK packet";
            return {};
        }
        
        // Send ACK(0) to acknowledge OACK
        TftpAckPacket ack0(0);
        std::vector<uint8_t> ack_data = ack0.serialize();
        if (!sendPacket(ack_data)) {
            last_error_ = "Failed to send ACK(0)";
            return {};
        }
        
        // Wait for first DATA packet
        response = receivePacket(std::chrono::milliseconds(transfer_timeout_ * 1000));
        if (response.empty()) {
            last_error_ = "Timeout waiting for DATA";
            return {};
        }
        opcode = (response[0] << 8) | response[1];
    }
    
    // Handle ERROR
    if (opcode == 5) {
        uint16_t error_code;
        std::string error_msg;
        if (handleError(response, error_code, error_msg)) {
            last_error_ = "Server error: " + error_msg;
        } else {
            last_error_ = "Server error (code: " + std::to_string(error_code) + ")";
        }
        return {};
    }
    
    // Collect data packets
    std::vector<uint8_t> file_data;
    uint16_t expected_block = 1;
    std::map<uint16_t, std::vector<uint8_t>> received_blocks;
    
    while (opcode == 3) { // DATA
        TftpDataPacket data_packet(response.data(), response.size());
        if (!data_packet.isValid()) {
            last_error_ = "Invalid DATA packet";
            return {};
        }
        
        uint16_t block_num = data_packet.getBlockNumber();
        std::vector<uint8_t> block_data = data_packet.getData();
        
        // Handle windowed transfers
        if (window_size_ > 1) {
            received_blocks[block_num] = block_data;
            
            // Send ACK for all blocks in window
            uint16_t window_start = expected_block;
            uint16_t window_end = window_start + window_size_ - 1;
            
            if (block_num >= window_start && block_num <= window_end) {
                // Send ACK for this block
                TftpAckPacket ack(block_num);
                std::vector<uint8_t> ack_data = ack.serialize();
                if (!sendPacket(ack_data)) {
                    last_error_ = "Failed to send ACK";
                    return {};
                }
                
                // Process blocks in order
                while (received_blocks.find(expected_block) != received_blocks.end()) {
                    const auto& block = received_blocks[expected_block];
                    file_data.insert(file_data.end(), block.begin(), block.end());
                    received_blocks.erase(expected_block);
                    expected_block++;
                    
                    // Check if this was the last block
                    if (block.size() < block_size_) {
                        last_success_ = true;
                        return file_data;
                    }
                }
            }
        } else {
            // Simple block-by-block transfer
            if (block_num == expected_block) {
                file_data.insert(file_data.end(), block_data.begin(), block_data.end());
                
                // Send ACK
                TftpAckPacket ack(block_num);
                std::vector<uint8_t> ack_data = ack.serialize();
                if (!sendPacket(ack_data)) {
                    last_error_ = "Failed to send ACK";
                    return {};
                }
                
                expected_block++;
                
                // Check if this was the last block
                if (block_data.size() < block_size_) {
                    last_success_ = true;
                    return file_data;
                }
            } else {
                // Out of order or duplicate - resend ACK
                TftpAckPacket ack(expected_block - 1);
                std::vector<uint8_t> ack_data = ack.serialize();
                sendPacket(ack_data);
            }
        }
        
        // Receive next packet
        response = receivePacket(std::chrono::milliseconds(transfer_timeout_ * 1000));
        if (response.empty()) {
            last_error_ = "Timeout waiting for DATA";
            return {};
        }
        opcode = (response[0] << 8) | response[1];
    }
    
    if (opcode == 5) {
        uint16_t error_code;
        std::string error_msg;
        if (handleError(response, error_code, error_msg)) {
            last_error_ = "Server error: " + error_msg;
        }
        return {};
    }
    
    last_success_ = true;
    return file_data;
}

bool TftpClient::writeFile(const std::string& filename,
                           const std::vector<uint8_t>& data,
                           const std::string& mode,
                           const TftpOptions& options) {
    last_success_ = false;
    last_error_.clear();
    
    // Reset negotiated options
    block_size_ = 512;
    window_size_ = 1;
    transfer_timeout_ = timeout_;
    
    // Create WRQ packet
    TftpMode tftp_mode = stringToTftpMode(mode);
    TftpRequestPacket wrq(TftpOpcode::WRQ, filename, tftp_mode);
    wrq.setOptions(options);
    
    std::vector<uint8_t> packet_data = wrq.serialize();
    if (!sendPacket(packet_data)) {
        last_error_ = "Failed to send WRQ";
        return false;
    }
    
    // Receive response (OACK or ACK(0))
    std::vector<uint8_t> response = receivePacket(std::chrono::milliseconds(timeout_ * 1000));
    if (response.empty()) {
        last_error_ = "Timeout waiting for response";
        return false;
    }
    
    uint16_t opcode = (response[0] << 8) | response[1];
    
    // Handle OACK
    TftpOptions negotiated_options;
    if (opcode == 6) { // OACK
        if (!handleOack(response, negotiated_options)) {
            last_error_ = "Invalid OACK packet";
            return false;
        }
        
        // Send ACK(0) to acknowledge OACK
        TftpAckPacket ack0(0);
        std::vector<uint8_t> ack_data = ack0.serialize();
        if (!sendPacket(ack_data)) {
            last_error_ = "Failed to send ACK(0)";
            return false;
        }
    } else if (opcode != 4) { // Not ACK
        if (opcode == 5) {
            uint16_t error_code;
            std::string error_msg;
            if (handleError(response, error_code, error_msg)) {
                last_error_ = "Server error: " + error_msg;
            }
        } else {
            last_error_ = "Unexpected packet type";
        }
        return false;
    }
    
    // Send data blocks
    uint16_t block_num = 1;
    size_t offset = 0;
    
    while (offset < data.size()) {
        size_t block_len = std::min<size_t>(block_size_, data.size() - offset);
        std::vector<uint8_t> block_data(data.begin() + offset, data.begin() + offset + block_len);
        
        TftpDataPacket data_packet(block_num, block_data);
        std::vector<uint8_t> packet = data_packet.serialize();
        
        if (!sendPacket(packet)) {
            last_error_ = "Failed to send DATA packet";
            return false;
        }
        
        // Wait for ACK
        response = receivePacket(std::chrono::milliseconds(transfer_timeout_ * 1000));
        if (response.empty()) {
            last_error_ = "Timeout waiting for ACK";
            return false;
        }
        
        opcode = (response[0] << 8) | response[1];
        if (opcode == 5) {
            uint16_t error_code;
            std::string error_msg;
            if (handleError(response, error_code, error_msg)) {
                last_error_ = "Server error: " + error_msg;
            }
            return false;
        }
        
        if (opcode != 4) {
            last_error_ = "Expected ACK, got opcode " + std::to_string(opcode);
            return false;
        }
        
        TftpAckPacket ack(response.data(), response.size());
        if (!ack.isValid() || ack.getBlockNumber() != block_num) {
            last_error_ = "Invalid or unexpected ACK";
            return false;
        }
        
        offset += block_len;
        block_num++;
        
        // Last block if it's smaller than block_size
        if (block_len < block_size_) {
            break;
        }
    }
    
    last_success_ = true;
    return true;
}

} // namespace test
} // namespace simple_tftpd

