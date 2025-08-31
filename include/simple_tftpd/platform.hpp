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

#include <string>
#include <cstdint>

namespace simple_tftpd {

/**
 * @brief Platform abstraction layer
 * 
 * This header provides platform-specific definitions and abstractions
 * for the TFTP daemon to work across different operating systems.
 */

#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    
    // Windows-specific types
    using socket_t = SOCKET;
    using ssize_t = SSIZE_T;
    
    // Windows socket error handling
    #define SOCKET_ERROR_CODE WSAGetLastError()
    #define CLOSE_SOCKET closesocket
    
    // Windows-specific constants
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    
    // Unix-like types
    using socket_t = int;
    
    // Unix socket error handling
    #define SOCKET_ERROR_CODE errno
    #define CLOSE_SOCKET close
    
    // Unix-specific constants
    #define INVALID_SOCKET_VALUE -1
    
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    
    // Unix-like types
    using socket_t = int;
    
    // Unix socket error handling
    #define SOCKET_ERROR_CODE errno
    #define CLOSE_SOCKET close
    
    // Unix-specific constants
    #define INVALID_SOCKET_VALUE -1
    
#else
    #error "Unsupported platform"
#endif

// Common platform-independent types
using port_t = uint16_t;
using ipv4_addr_t = uint32_t;
using ipv6_addr_t = uint8_t[16];

// Platform-independent constants
constexpr port_t TFTP_DEFAULT_PORT = 69;
constexpr size_t TFTP_MAX_PACKET_SIZE = 512;
constexpr size_t TFTP_MAX_FILENAME_LENGTH = 512;
constexpr size_t TFTP_MAX_MODE_LENGTH = 10;

// Platform-independent error codes
enum class TftpError {
    SUCCESS = 0,
    INVALID_PACKET = 1,
    FILE_NOT_FOUND = 2,
    ACCESS_VIOLATION = 3,
    DISK_FULL = 4,
    ILLEGAL_OPERATION = 5,
    UNKNOWN_TRANSFER_ID = 6,
    FILE_EXISTS = 7,
    NO_SUCH_USER = 8,
    TIMEOUT = 9,
    NETWORK_ERROR = 10,
    PLATFORM_ERROR = 11
};

// Platform-independent TFTP operation codes
enum class TftpOpcode : uint16_t {
    RRQ = 1,    // Read Request
    WRQ = 2,    // Write Request
    DATA = 3,   // Data
    ACK = 4,    // Acknowledgment
    ERROR = 5,  // Error
    OACK = 6    // Option Acknowledgment
};

// Platform-independent TFTP modes
enum class TftpMode {
    NETASCII,
    OCTET,
    MAIL
};

// Platform-independent TFTP options
struct TftpOptions {
    uint16_t blksize = 512;
    uint16_t timeout = 5;
    uint16_t tsize = 0;
    uint16_t windowsize = 1;
    bool multicast = false;
    std::string multicast_ip;
    uint16_t multicast_port = 0;
};

} // namespace simple_tftpd
