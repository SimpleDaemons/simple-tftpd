/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "simple-tftpd/utils/platform.hpp"
#include "simple-tftpd/core/packet.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <chrono>

namespace simple_tftpd {
namespace test {

/**
 * @brief Simple TFTP client for integration testing
 */
class TftpClient {
public:
    TftpClient(const std::string& server_addr, port_t server_port);
    ~TftpClient();
    
    /**
     * @brief Read a file from the server
     * @param filename Remote filename
     * @param mode Transfer mode (netascii, octet, mail)
     * @param options TFTP options to request
     * @return File contents
     */
    std::vector<uint8_t> readFile(const std::string& filename, 
                                   const std::string& mode = "octet",
                                   const TftpOptions& options = TftpOptions());
    
    /**
     * @brief Write a file to the server
     * @param filename Remote filename
     * @param data File data
     * @param mode Transfer mode (netascii, octet, mail)
     * @param options TFTP options to request
     * @return true if successful
     */
    bool writeFile(const std::string& filename,
                   const std::vector<uint8_t>& data,
                   const std::string& mode = "octet",
                   const TftpOptions& options = TftpOptions());
    
    /**
     * @brief Set receive timeout
     * @param timeout Timeout in seconds
     */
    void setTimeout(std::chrono::seconds timeout);
    
    /**
     * @brief Get last error message
     */
    std::string getLastError() const { return last_error_; }
    
    /**
     * @brief Check if last operation was successful
     */
    bool isSuccess() const { return last_success_; }

private:
    bool initializeSocket();
    void closeSocket();
    
    bool sendPacket(const std::vector<uint8_t>& packet);
    std::vector<uint8_t> receivePacket(std::chrono::milliseconds timeout);
    
    bool handleOack(const std::vector<uint8_t>& packet, TftpOptions& negotiated_options);
    bool handleError(const std::vector<uint8_t>& packet, uint16_t& error_code, std::string& error_msg);
    
    std::string server_addr_;
    port_t server_port_;
    socket_t client_socket_;
    port_t client_port_;
    std::string last_error_;
    bool last_success_;
    std::chrono::seconds timeout_;
    
    // Negotiated options
    uint16_t block_size_;
    uint16_t window_size_;
    std::chrono::seconds transfer_timeout_;
};

} // namespace test
} // namespace simple_tftpd

