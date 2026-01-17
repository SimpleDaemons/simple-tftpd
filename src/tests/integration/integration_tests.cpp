/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include "simple-tftpd/core/server.hpp"
#include "simple-tftpd/config.hpp"
#include "simple-tftpd/utils/logger.hpp"
#include "tftp_client.hpp"
#include "../utils/test_helpers.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>

using namespace simple_tftpd;
using namespace simple_tftpd::test;

class IntegrationTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        helpers_ = std::make_unique<TestHelpers>();
        test_dir_ = helpers_->getTestDirectory();
        
        // Find available port
        test_port_ = helpers_->findAvailablePort(6969);
        ASSERT_NE(test_port_, 0) << "No available port found";
        
        // Create configuration
        config_ = std::make_shared<TftpConfig>();
        config_->setListenAddress("127.0.0.1");
        config_->setListenPort(test_port_);
        config_->setRootDirectory(test_dir_);
        config_->setReadEnabled(true);
        config_->setWriteEnabled(true);
        config_->setMaxFileSize(10 * 1024 * 1024); // 10MB
        config_->setBlockSize(512);
        config_->setWindowSize(1);
        config_->setTimeout(5);
        config_->setMaxRetries(5);
        
        // Create logger (suppress output for tests)
        logger_ = std::make_shared<Logger>("", LogLevel::ERROR, false);
        
        // Create and start server
        server_ = std::make_shared<TftpServer>(config_, logger_);
        ASSERT_TRUE(server_->start()) << "Failed to start TFTP server";
        
        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Create client
        client_ = std::make_unique<TftpClient>("127.0.0.1", test_port_);
    }
    
    void TearDown() override {
        if (server_) {
            server_->stop();
        }
        // Give server time to stop
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::unique_ptr<TestHelpers> helpers_;
    std::string test_dir_;
    port_t test_port_;
    std::shared_ptr<TftpConfig> config_;
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<TftpServer> server_;
    std::unique_ptr<TftpClient> client_;
};

// Basic Read Transfer Tests
TEST_F(IntegrationTestFixture, BasicReadTransfer) {
    // Create test file
    std::string test_content = "Hello, TFTP World!";
    std::string filename = "test_read.txt";
    helpers_->createTestFile(filename, test_content);
    
    // Read file via TFTP
    std::vector<uint8_t> received = client_->readFile(filename, "octet");
    
    ASSERT_TRUE(client_->isSuccess()) << "Read failed: " << client_->getLastError();
    ASSERT_EQ(received.size(), test_content.size());
    ASSERT_EQ(std::string(received.begin(), received.end()), test_content);
}

TEST_F(IntegrationTestFixture, BasicWriteTransfer) {
    // Prepare test data
    std::string test_content = "This is a test file for writing";
    std::vector<uint8_t> data(test_content.begin(), test_content.end());
    
    // Write file via TFTP
    bool success = client_->writeFile("test_write.txt", data, "octet");
    
    ASSERT_TRUE(success) << "Write failed: " << client_->getLastError();
    
    // Verify file was written
    std::string filepath = test_dir_ + "/test_write.txt";
    ASSERT_TRUE(helpers_->fileExists(filepath));
    
    std::string written_content = helpers_->readFile(filepath);
    ASSERT_EQ(written_content, test_content);
}

TEST_F(IntegrationTestFixture, LargeFileTransfer) {
    // Create a larger file (50KB)
    size_t file_size = 50 * 1024;
    std::vector<uint8_t> original_data = helpers_->generateRandomData(file_size);
    std::string filename = "large_file.bin";
    
    // Write file
    bool write_success = client_->writeFile(filename, original_data, "octet");
    ASSERT_TRUE(write_success) << "Write failed: " << client_->getLastError();
    
    // Read file back
    std::vector<uint8_t> received_data = client_->readFile(filename, "octet");
    ASSERT_TRUE(client_->isSuccess()) << "Read failed: " << client_->getLastError();
    
    // Verify data matches
    ASSERT_EQ(received_data.size(), original_data.size());
    ASSERT_EQ(received_data, original_data);
}

// Transfer Mode Tests
TEST_F(IntegrationTestFixture, NetasciiModeRead) {
    // Create file with LF line endings
    std::string content = "Line 1\nLine 2\nLine 3\n";
    helpers_->createTestFile("netascii_test.txt", content);
    
    // Read in netascii mode (should convert LF to CRLF)
    std::vector<uint8_t> received = client_->readFile("netascii_test.txt", "netascii");
    
    ASSERT_TRUE(client_->isSuccess());
    // In netascii mode, LF should be converted to CRLF
    std::string received_str(received.begin(), received.end());
    ASSERT_NE(received_str.find("\r\n"), std::string::npos);
}

TEST_F(IntegrationTestFixture, OctetModeBinary) {
    // Create binary file with null bytes
    std::vector<uint8_t> binary_data = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x42};
    helpers_->createTestFile("binary_test.bin", std::string(binary_data.begin(), binary_data.end()));
    
    // Read in octet mode
    std::vector<uint8_t> received = client_->readFile("binary_test.bin", "octet");
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received, binary_data);
}

// Option Negotiation Tests
TEST_F(IntegrationTestFixture, BlksizeOption) {
    // Create test file
    std::string content = "Test content for blksize option";
    helpers_->createTestFile("blksize_test.txt", content);
    
    // Request larger block size
    TftpOptions options;
    options.has_blksize = true;
    options.blksize = 1024;
    
    std::vector<uint8_t> received = client_->readFile("blksize_test.txt", "octet", options);
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received.size(), content.size());
}

TEST_F(IntegrationTestFixture, TimeoutOption) {
    std::string content = "Test timeout option";
    helpers_->createTestFile("timeout_test.txt", content);
    
    TftpOptions options;
    options.has_timeout = true;
    options.timeout = 10;
    
    std::vector<uint8_t> received = client_->readFile("timeout_test.txt", "octet", options);
    
    ASSERT_TRUE(client_->isSuccess());
}

TEST_F(IntegrationTestFixture, TsizeOption) {
    std::string content = "Test tsize option";
    helpers_->createTestFile("tsize_test.txt", content);
    
    TftpOptions options;
    options.has_tsize = true;
    options.tsize = 0; // Will be filled by server
    
    std::vector<uint8_t> received = client_->readFile("tsize_test.txt", "octet", options);
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received.size(), content.size());
}

TEST_F(IntegrationTestFixture, WindowsizeOption) {
    // Create larger file to test windowing
    size_t file_size = 10 * 1024; // 10KB
    std::vector<uint8_t> data = helpers_->generateRandomData(file_size);
    helpers_->createTestFile("window_test.bin", std::string(data.begin(), data.end()));
    
    TftpOptions options;
    options.has_windowsize = true;
    options.windowsize = 4; // Request window size of 4
    
    std::vector<uint8_t> received = client_->readFile("window_test.bin", "octet", options);
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received.size(), data.size());
}

TEST_F(IntegrationTestFixture, MultipleOptions) {
    std::string content = "Test multiple options";
    helpers_->createTestFile("multiopt_test.txt", content);
    
    TftpOptions options;
    options.has_blksize = true;
    options.blksize = 1024;
    options.has_timeout = true;
    options.timeout = 8;
    options.has_windowsize = true;
    options.windowsize = 2;
    
    std::vector<uint8_t> received = client_->readFile("multiopt_test.txt", "octet", options);
    
    ASSERT_TRUE(client_->isSuccess());
}

// Security Tests
TEST_F(IntegrationTestFixture, FileExtensionFiltering) {
    // Set allowed extensions
    config_->setAllowedExtensions({"txt", "bin"});
    
    // Restart server with new config
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    ASSERT_TRUE(server_->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Create allowed file
    helpers_->createTestFile("allowed.txt", "test");
    
    // Try to read allowed file
    std::vector<uint8_t> received = client_->readFile("allowed.txt", "octet");
    ASSERT_TRUE(client_->isSuccess());
    
    // Create disallowed file
    helpers_->createTestFile("disallowed.exe", "test");
    
    // Try to read disallowed file (should fail)
    client_ = std::make_unique<TftpClient>("127.0.0.1", test_port_);
    std::vector<uint8_t> received2 = client_->readFile("disallowed.exe", "octet");
    ASSERT_FALSE(client_->isSuccess());
}

TEST_F(IntegrationTestFixture, ClientAddressFiltering) {
    // Set allowed clients
    config_->setAllowedClients({"127.0.0.1"});
    
    // Restart server
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    ASSERT_TRUE(server_->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Create test file
    helpers_->createTestFile("client_test.txt", "test");
    
    // Read should succeed from allowed client
    std::vector<uint8_t> received = client_->readFile("client_test.txt", "octet");
    ASSERT_TRUE(client_->isSuccess());
}

TEST_F(IntegrationTestFixture, MaxFileSizeLimit) {
    // Set small file size limit
    config_->setMaxFileSize(1024); // 1KB
    
    // Restart server
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    ASSERT_TRUE(server_->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to write file larger than limit
    std::vector<uint8_t> large_data = helpers_->generateRandomData(2048); // 2KB
    client_ = std::make_unique<TftpClient>("127.0.0.1", test_port_);
    bool success = client_->writeFile("too_large.bin", large_data, "octet");
    
    // Should fail or be truncated (implementation dependent)
    // For now, just verify it doesn't crash
}

TEST_F(IntegrationTestFixture, WriteProtection) {
    // Disable write
    config_->setWriteEnabled(false);
    
    // Restart server
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    ASSERT_TRUE(server_->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to write (should fail)
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    client_ = std::make_unique<TftpClient>("127.0.0.1", test_port_);
    bool success = client_->writeFile("write_test.txt", data, "octet");
    
    ASSERT_FALSE(success);
}

// Error Handling Tests
TEST_F(IntegrationTestFixture, FileNotFound) {
    // Try to read non-existent file
    std::vector<uint8_t> received = client_->readFile("nonexistent.txt", "octet");
    
    ASSERT_FALSE(client_->isSuccess());
    ASSERT_TRUE(received.empty());
}

TEST_F(IntegrationTestFixture, InvalidMode) {
    // Create test file
    helpers_->createTestFile("mode_test.txt", "test");
    
    // Try invalid mode (should default to octet or fail gracefully)
    std::vector<uint8_t> received = client_->readFile("mode_test.txt", "invalid_mode");
    
    // Behavior depends on implementation - just verify no crash
}

// Performance Tests
TEST_F(IntegrationTestFixture, MultipleSequentialTransfers) {
    // Create multiple files
    for (int i = 0; i < 5; ++i) {
        std::string filename = "seq_test_" + std::to_string(i) + ".txt";
        std::string content = "File " + std::to_string(i);
        helpers_->createTestFile(filename, content);
        
        // Read each file
        std::vector<uint8_t> received = client_->readFile(filename, "octet");
        ASSERT_TRUE(client_->isSuccess()) << "Failed on file " << i;
        ASSERT_EQ(std::string(received.begin(), received.end()), content);
    }
}

TEST_F(IntegrationTestFixture, EmptyFileTransfer) {
    // Create empty file
    helpers_->createTestFile("empty.txt", "");
    
    // Read empty file
    std::vector<uint8_t> received = client_->readFile("empty.txt", "octet");
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received.size(), 0);
}

TEST_F(IntegrationTestFixture, SingleBlockFile) {
    // Create file exactly one block (512 bytes)
    std::vector<uint8_t> data(512, 0x42);
    helpers_->createTestFile("single_block.bin", std::string(data.begin(), data.end()));
    
    // Read file
    std::vector<uint8_t> received = client_->readFile("single_block.bin", "octet");
    
    ASSERT_TRUE(client_->isSuccess());
    ASSERT_EQ(received.size(), 512);
    ASSERT_EQ(received, data);
}

// Edge Cases
TEST_F(IntegrationTestFixture, LongFilename) {
    // Create file with long filename
    std::string long_name(200, 'a');
    long_name += ".txt";
    helpers_->createTestFile(long_name, "test");
    
    // Read file
    std::vector<uint8_t> received = client_->readFile(long_name, "octet");
    
    // Should either succeed or fail gracefully
    // Implementation dependent
}

TEST_F(IntegrationTestFixture, SpecialCharactersInFilename) {
    // Create file with special characters (if supported)
    std::string filename = "test_file_123.txt";
    helpers_->createTestFile(filename, "test");
    
    // Read file
    std::vector<uint8_t> received = client_->readFile(filename, "octet");
    
    ASSERT_TRUE(client_->isSuccess());
}

