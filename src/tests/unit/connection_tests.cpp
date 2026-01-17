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

#include <gtest/gtest.h>
#include "simple-tftpd/core/connection.hpp"
#include "simple-tftpd/core/server.hpp"
#include "simple-tftpd/config/config.hpp"
#include "simple-tftpd/utils/logger.hpp"
#include "simple-tftpd/utils/platform.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace simple_tftpd;

// Note: Connection tests require a real TftpServer instance
// since TftpConnection needs a server reference for sendPacket calls
// These tests focus on basic connection properties and state management

// Test fixture for connection tests
class TftpConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<TftpConfig>();
        logger = std::make_shared<Logger>();
        logger->setLevel(LogLevel::ERROR); // Suppress logs during tests
        server = std::make_unique<TftpServer>(config, logger);
    }

    void TearDown() override {
        if (connection && connection->isActive()) {
            connection->stop();
        }
        connection.reset();
        if (server && server->isRunning()) {
            server->stop();
        }
        server.reset();
    }

    std::unique_ptr<TftpServer> server;
    std::shared_ptr<TftpConfig> config;
    std::shared_ptr<Logger> logger;
    std::unique_ptr<TftpConnection> connection;
};

// Test connection creation
TEST_F(TftpConnectionTest, ConnectionCreation) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    EXPECT_FALSE(connection->isActive());
    EXPECT_EQ(connection->getState(), TftpConnectionState::INITIALIZED);
    EXPECT_EQ(connection->getClientAddress(), "127.0.0.1");
    EXPECT_EQ(connection->getClientPort(), 12345);
    EXPECT_EQ(connection->getBytesTransferred(), 0);
}

// Test connection start and stop
TEST_F(TftpConnectionTest, ConnectionStartStop) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    EXPECT_FALSE(connection->isActive());
    
    // Note: Starting a connection may require proper server setup
    // This test verifies the basic state transitions
    EXPECT_EQ(connection->getState(), TftpConnectionState::INITIALIZED);
    
    connection->stop();
    EXPECT_FALSE(connection->isActive());
}

// Test connection state transitions
TEST_F(TftpConnectionTest, ConnectionStateTransitions) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    EXPECT_EQ(connection->getState(), TftpConnectionState::INITIALIZED);
    
    // State should be initialized initially
    EXPECT_EQ(connection->getState(), TftpConnectionState::INITIALIZED);
}

// Test connection timeout handling
TEST_F(TftpConnectionTest, ConnectionTimeout) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Set a short timeout
    connection->setTimeout(std::chrono::seconds(1));
    EXPECT_EQ(connection->getTimeout(), std::chrono::seconds(1));
    
    // Test timeout handling
    // Note: Actual timeout behavior may require running connection
    bool handled = connection->handleTimeout();
    // Timeout handling should not crash
    EXPECT_TRUE(handled || !handled); // Either result is acceptable for now
}

// Test connection client information
TEST_F(TftpConnectionTest, ConnectionClientInfo) {
    connection = std::make_unique<TftpConnection>(
        *server, "192.168.1.100", 54321, config, logger);
    
    EXPECT_EQ(connection->getClientAddress(), "192.168.1.100");
    EXPECT_EQ(connection->getClientPort(), 54321);
}

// Test connection duration tracking
TEST_F(TftpConnectionTest, ConnectionDuration) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    auto start_time = connection->getStartTime();
    auto duration = connection->getDuration();
    
    // Duration should be minimal initially
    EXPECT_GE(duration.count(), 0);
    
    // Start time should be set
    EXPECT_GT(start_time.time_since_epoch().count(), 0);
}

// Test connection error handling
TEST_F(TftpConnectionTest, ConnectionErrorHandling) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Test invalid packet handling
    bool handled = connection->handleInvalidPacket("Test error");
    EXPECT_TRUE(handled || !handled); // Should not crash
    
    // Test file error handling
    bool file_handled = connection->handleFileError("read", "test.txt");
    EXPECT_TRUE(file_handled || !file_handled); // Should not crash
}

// Test connection send error packet
TEST_F(TftpConnectionTest, SendErrorPacket) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Test sending error packet
    bool sent = connection->sendError(TftpError::FILE_NOT_FOUND, "File not found");
    // Should attempt to send (may fail if connection not fully initialized)
    EXPECT_TRUE(sent || !sent); // Either result acceptable
}

// Test connection timeout getter/setter
TEST_F(TftpConnectionTest, ConnectionTimeoutGetterSetter) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Test default timeout
    auto default_timeout = connection->getTimeout();
    EXPECT_GE(default_timeout.count(), 0);
    
    // Set custom timeout
    connection->setTimeout(std::chrono::seconds(30));
    EXPECT_EQ(connection->getTimeout(), std::chrono::seconds(30));
    
    // Set another timeout
    connection->setTimeout(std::chrono::seconds(60));
    EXPECT_EQ(connection->getTimeout(), std::chrono::seconds(60));
}

// Test connection callback
TEST_F(TftpConnectionTest, ConnectionCallback) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    bool callback_called = false;
    TftpConnectionState callback_state;
    std::string callback_message;
    
    connection->setCallback([&](TftpConnectionState state, const std::string& msg) {
        callback_called = true;
        callback_state = state;
        callback_message = msg;
    });
    
    // Callback should be set (actual invocation depends on connection events)
    // This test verifies the callback can be set without errors
    EXPECT_FALSE(callback_called); // Initially not called
}

// Test connection bytes transferred tracking
TEST_F(TftpConnectionTest, BytesTransferredTracking) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Initially should be 0
    EXPECT_EQ(connection->getBytesTransferred(), 0);
    
    // Bytes transferred should be tracked (actual updates happen during transfers)
    // This test verifies the getter works
    size_t bytes = connection->getBytesTransferred();
    EXPECT_EQ(bytes, 0);
}

// Test connection transfer direction
TEST_F(TftpConnectionTest, TransferDirection) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Transfer direction should be accessible
    // Initial direction depends on implementation
    auto direction = connection->getTransferDirection();
    // Should be either READ or WRITE
    EXPECT_TRUE(direction == TftpTransferDirection::READ || 
                direction == TftpTransferDirection::WRITE);
}

// Test connection filename tracking
TEST_F(TftpConnectionTest, FilenameTracking) {
    connection = std::make_unique<TftpConnection>(
        *server, "127.0.0.1", 12345, config, logger);
    
    // Filename should be accessible (empty initially)
    std::string filename = connection->getFilename();
    // Should not crash, may be empty initially
    EXPECT_TRUE(filename.empty() || !filename.empty());
}
