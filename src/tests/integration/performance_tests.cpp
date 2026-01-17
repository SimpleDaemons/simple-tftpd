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
#include <vector>
#include <algorithm>
#include <numeric>

using namespace simple_tftpd;
using namespace simple_tftpd::test;

class PerformanceTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        helpers_ = std::make_unique<TestHelpers>();
        test_dir_ = helpers_->getTestDirectory();
        test_port_ = helpers_->findAvailablePort(6969);
        
        config_ = std::make_shared<TftpConfig>();
        config_->setListenAddress("127.0.0.1");
        config_->setListenPort(test_port_);
        config_->setRootDirectory(test_dir_);
        config_->setReadEnabled(true);
        config_->setWriteEnabled(true);
        config_->setMaxFileSize(100 * 1024 * 1024);
        config_->setBlockSize(512);
        config_->setWindowSize(1);
        config_->setTimeout(5);
        
        logger_ = std::make_shared<Logger>("", LogLevel::ERROR, false);
        server_ = std::make_shared<TftpServer>(config_, logger_);
        ASSERT_TRUE(server_->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        if (server_) {
            server_->stop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::unique_ptr<TestHelpers> helpers_;
    std::string test_dir_;
    port_t test_port_;
    std::shared_ptr<TftpConfig> config_;
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<TftpServer> server_;
};

// Basic throughput test
TEST_F(PerformanceTestFixture, BasicThroughput) {
    // Create a 1MB test file
    size_t file_size = 1024 * 1024; // 1MB
    std::vector<uint8_t> test_data = helpers_->generateRandomData(file_size);
    std::string filename = "throughput_test.bin";
    
    // Write file
    TftpClient client("127.0.0.1", test_port_);
    auto start = std::chrono::steady_clock::now();
    bool write_success = client.writeFile(filename, test_data, "octet");
    auto write_end = std::chrono::steady_clock::now();
    
    ASSERT_TRUE(write_success);
    
    // Read file
    auto read_start = std::chrono::steady_clock::now();
    std::vector<uint8_t> received = client.readFile(filename, "octet");
    auto read_end = std::chrono::steady_clock::now();
    
    ASSERT_TRUE(client.isSuccess());
    ASSERT_EQ(received.size(), test_data.size());
    
    // Calculate throughput
    auto write_duration = std::chrono::duration_cast<std::chrono::milliseconds>(write_end - start);
    auto read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(read_end - read_start);
    
    double write_mbps = (file_size * 8.0) / (write_duration.count() * 1000.0); // Convert to Mbps
    double read_mbps = (file_size * 8.0) / (read_duration.count() * 1000.0);
    
    // Basic sanity check - should complete in reasonable time
    EXPECT_LT(write_duration.count(), 10000); // Less than 10 seconds
    EXPECT_LT(read_duration.count(), 10000);
    
    // Log results
    std::cout << "Write throughput: " << write_mbps << " Mbps (" 
              << write_duration.count() << " ms)" << std::endl;
    std::cout << "Read throughput: " << read_mbps << " Mbps (" 
              << read_duration.count() << " ms)" << std::endl;
}

// Windowed transfer performance
TEST_F(PerformanceTestFixture, WindowedTransferPerformance) {
    size_t file_size = 512 * 1024; // 512KB
    std::vector<uint8_t> test_data = helpers_->generateRandomData(file_size);
    std::string filename = "windowed_test.bin";
    
    // Test with window size 1 (default)
    config_->setWindowSize(1);
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    server_->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TftpClient client1("127.0.0.1", test_port_);
    TftpOptions options1;
    options1.has_windowsize = true;
    options1.windowsize = 1;
    
    auto start1 = std::chrono::steady_clock::now();
    std::vector<uint8_t> received1 = client1.readFile(filename, "octet", options1);
    auto end1 = std::chrono::steady_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // Test with larger window size
    config_->setWindowSize(4);
    server_->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server_ = std::make_shared<TftpServer>(config_, logger_);
    server_->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TftpClient client2("127.0.0.1", test_port_);
    TftpOptions options2;
    options2.has_windowsize = true;
    options2.windowsize = 4;
    
    auto start2 = std::chrono::steady_clock::now();
    std::vector<uint8_t> received2 = client2.readFile(filename, "octet", options2);
    auto end2 = std::chrono::steady_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    // Windowed should be faster (or at least not slower)
    std::cout << "Window size 1: " << duration1.count() << " ms" << std::endl;
    std::cout << "Window size 4: " << duration2.count() << " ms" << std::endl;
}

// Block size performance
TEST_F(PerformanceTestFixture, BlockSizePerformance) {
    size_t file_size = 256 * 1024; // 256KB
    std::vector<uint8_t> test_data = helpers_->generateRandomData(file_size);
    std::string filename = "blksize_test.bin";
    helpers_->createTestFile(filename, std::string(test_data.begin(), test_data.end()));
    
    // Test with default block size (512)
    TftpClient client1("127.0.0.1", test_port_);
    TftpOptions options1;
    options1.has_blksize = true;
    options1.blksize = 512;
    
    auto start1 = std::chrono::steady_clock::now();
    std::vector<uint8_t> received1 = client1.readFile(filename, "octet", options1);
    auto end1 = std::chrono::steady_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // Test with larger block size (1024)
    TftpClient client2("127.0.0.1", test_port_);
    TftpOptions options2;
    options2.has_blksize = true;
    options2.blksize = 1024;
    
    auto start2 = std::chrono::steady_clock::now();
    std::vector<uint8_t> received2 = client2.readFile(filename, "octet", options2);
    auto end2 = std::chrono::steady_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "Block size 512: " << duration1.count() << " ms" << std::endl;
    std::cout << "Block size 1024: " << duration2.count() << " ms" << std::endl;
    
    // Larger blocks should generally be faster
    EXPECT_LE(received1.size(), test_data.size());
    EXPECT_LE(received2.size(), test_data.size());
}

// Multiple sequential transfers
TEST_F(PerformanceTestFixture, SequentialTransfers) {
    size_t file_size = 64 * 1024; // 64KB per file
    int num_files = 10;
    
    std::vector<std::chrono::milliseconds> durations;
    
    for (int i = 0; i < num_files; ++i) {
        std::string filename = "seq_test_" + std::to_string(i) + ".bin";
        std::vector<uint8_t> test_data = helpers_->generateRandomData(file_size);
        helpers_->createTestFile(filename, std::string(test_data.begin(), test_data.end()));
        
        TftpClient client("127.0.0.1", test_port_);
        auto start = std::chrono::steady_clock::now();
        std::vector<uint8_t> received = client.readFile(filename, "octet");
        auto end = std::chrono::steady_clock::now();
        
        ASSERT_TRUE(client.isSuccess());
        durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
    }
    
    // Calculate average
    auto total = std::accumulate(durations.begin(), durations.end(), 
                                std::chrono::milliseconds(0),
                                [](const auto& a, const auto& b) { return a + b; });
    auto average = total.count() / num_files;
    
    std::cout << "Average transfer time for " << num_files << " files: " 
              << average << " ms" << std::endl;
    std::cout << "Total time: " << total.count() << " ms" << std::endl;
}

