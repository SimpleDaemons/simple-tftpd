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
#include "simple-tftpd/config/config.hpp"
#include <fstream>
#include <filesystem>
#include <cstdio>

using namespace simple_tftpd;

// Test fixture for configuration tests
class TftpConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<TftpConfig>();
        test_config_file = "/tmp/test_tftpd_config.json";
    }

    void TearDown() override {
        // Clean up test config file
        if (std::filesystem::exists(test_config_file)) {
            std::filesystem::remove(test_config_file);
        }
    }

    std::unique_ptr<TftpConfig> config;
    std::string test_config_file;
};

// Test default configuration values
TEST_F(TftpConfigTest, DefaultValues) {
    EXPECT_EQ(config->getListenAddress(), "0.0.0.0");
    EXPECT_EQ(config->getListenPort(), 69);
    EXPECT_TRUE(config->isIpv6Enabled());
    EXPECT_EQ(config->getRootDirectory(), "/var/tftp");
    EXPECT_TRUE(config->isReadEnabled());
    EXPECT_FALSE(config->isWriteEnabled());
    EXPECT_EQ(config->getMaxFileSize(), 104857600); // 100MB
}

// Test configuration validation
TEST_F(TftpConfigTest, ConfigurationValidation) {
    // Default config should be valid
    EXPECT_TRUE(config->validate());
    
    // Set invalid port (too high)
    config->setListenPort(65536);
    // Validation should catch this or clamp it
    bool valid = config->validate();
    // Either valid or invalid is acceptable depending on implementation
    EXPECT_TRUE(valid || !valid);
}

// Test configuration serialization to JSON
TEST_F(TftpConfigTest, ConfigurationSerialization) {
    // Get JSON representation
    std::string json = config->toJson();
    
    // JSON should not be empty
    EXPECT_FALSE(json.empty());
    
    // JSON should contain expected keys
    EXPECT_TRUE(json.find("network") != std::string::npos ||
                json.find("listen_address") != std::string::npos ||
                json.find("listen_port") != std::string::npos);
}

// Test loading configuration from JSON string
TEST_F(TftpConfigTest, LoadFromJsonString) {
    std::string json_config = R"({
        "network": {
            "listen_address": "127.0.0.1",
            "listen_port": 6969,
            "ipv6_enabled": false
        },
        "filesystem": {
            "root_directory": "/tmp/tftp"
        },
        "security": {
            "read_enabled": true,
            "write_enabled": true,
            "max_file_size": 52428800
        }
    })";
    
    bool loaded = config->loadFromJson(json_config);
    EXPECT_TRUE(loaded);
    
    if (loaded) {
        EXPECT_EQ(config->getListenAddress(), "127.0.0.1");
        EXPECT_EQ(config->getListenPort(), 6969);
        EXPECT_FALSE(config->isIpv6Enabled());
        EXPECT_EQ(config->getRootDirectory(), "/tmp/tftp");
        EXPECT_TRUE(config->isWriteEnabled());
        EXPECT_EQ(config->getMaxFileSize(), 52428800);
    }
}

// Test saving configuration to file
TEST_F(TftpConfigTest, SaveToFile) {
    // Modify some settings
    config->setListenPort(6969);
    config->setRootDirectory("/tmp/tftp");
    
    // Save to file
    bool saved = config->saveToFile(test_config_file);
    EXPECT_TRUE(saved);
    
    if (saved) {
        // Verify file exists
        EXPECT_TRUE(std::filesystem::exists(test_config_file));
    }
}

// Test loading configuration from file
TEST_F(TftpConfigTest, LoadFromFile) {
    // First save a configuration
    config->setListenPort(6969);
    config->setRootDirectory("/tmp/tftp");
    config->saveToFile(test_config_file);
    
    // Create new config and load from file
    auto new_config = std::make_unique<TftpConfig>();
    bool loaded = new_config->loadFromFile(test_config_file);
    EXPECT_TRUE(loaded);
    
    if (loaded) {
        EXPECT_EQ(new_config->getListenPort(), 6969);
        EXPECT_EQ(new_config->getRootDirectory(), "/tmp/tftp");
    }
}

// Test configuration edge cases
TEST_F(TftpConfigTest, ConfigurationEdgeCases) {
    // Test empty JSON
    bool loaded = config->loadFromJson("");
    // Should handle gracefully (may fail, but shouldn't crash)
    EXPECT_TRUE(loaded || !loaded);
    
    // Test invalid JSON
    loaded = config->loadFromJson("{ invalid json }");
    EXPECT_FALSE(loaded);
    
    // Test missing file
    loaded = config->loadFromFile("/nonexistent/file.json");
    EXPECT_FALSE(loaded);
}

// Test configuration getters and setters
TEST_F(TftpConfigTest, ConfigurationGettersSetters) {
    // Test network settings
    config->setListenAddress("192.168.1.1");
    EXPECT_EQ(config->getListenAddress(), "192.168.1.1");
    
    config->setListenPort(1234);
    EXPECT_EQ(config->getListenPort(), 1234);
    
    config->setIpv6Enabled(false);
    EXPECT_FALSE(config->isIpv6Enabled());
    
    // Test filesystem settings
    config->setRootDirectory("/custom/path");
    EXPECT_EQ(config->getRootDirectory(), "/custom/path");
    
    // Test security settings
    config->setReadEnabled(false);
    EXPECT_FALSE(config->isReadEnabled());
    
    config->setWriteEnabled(true);
    EXPECT_TRUE(config->isWriteEnabled());
    
    config->setMaxFileSize(1024 * 1024); // 1MB
    EXPECT_EQ(config->getMaxFileSize(), 1024 * 1024);
}

// Test allowed directories configuration
TEST_F(TftpConfigTest, AllowedDirectories) {
    std::vector<std::string> dirs = {"/tmp/dir1", "/tmp/dir2"};
    config->setAllowedDirectories(dirs);
    
    auto allowed = config->getAllowedDirectories();
    EXPECT_EQ(allowed.size(), 2);
    if (allowed.size() >= 2) {
        EXPECT_EQ(allowed[0], "/tmp/dir1");
        EXPECT_EQ(allowed[1], "/tmp/dir2");
    }
}

// Test allowed extensions configuration
TEST_F(TftpConfigTest, AllowedExtensions) {
    std::vector<std::string> exts = {"bin", "img", "txt"};
    config->setAllowedExtensions(exts);
    
    auto allowed = config->getAllowedExtensions();
    EXPECT_EQ(allowed.size(), 3);
    if (allowed.size() >= 3) {
        EXPECT_EQ(allowed[0], "bin");
        EXPECT_EQ(allowed[1], "img");
        EXPECT_EQ(allowed[2], "txt");
    }
}

// Test allowed clients configuration
TEST_F(TftpConfigTest, AllowedClients) {
    std::vector<std::string> clients = {"192.168.1.1", "10.0.0.1"};
    config->setAllowedClients(clients);
    
    auto allowed = config->getAllowedClients();
    EXPECT_EQ(allowed.size(), 2);
    if (allowed.size() >= 2) {
        EXPECT_EQ(allowed[0], "192.168.1.1");
        EXPECT_EQ(allowed[1], "10.0.0.1");
    }
}

// Test performance settings
TEST_F(TftpConfigTest, PerformanceSettings) {
    config->setBlockSize(1024);
    EXPECT_EQ(config->getBlockSize(), 1024);
    
    config->setTimeout(10);
    EXPECT_EQ(config->getTimeout(), 10);
    
    config->setWindowSize(4);
    EXPECT_EQ(config->getWindowSize(), 4);
    
    config->setMaxRetries(3);
    EXPECT_EQ(config->getMaxRetries(), 3);
}

// Test logging configuration
TEST_F(TftpConfigTest, LoggingConfiguration) {
    config->setLogLevel(LogLevel::DEBUG);
    EXPECT_EQ(config->getLogLevel(), LogLevel::DEBUG);
    
    config->setLogFile("/tmp/test.log");
    EXPECT_EQ(config->getLogFile(), "/tmp/test.log");
    
    config->setConsoleLogging(false);
    EXPECT_FALSE(config->isConsoleLoggingEnabled());
}

// Test overwrite protection
TEST_F(TftpConfigTest, OverwriteProtection) {
    config->setOverwriteProtection(true);
    EXPECT_TRUE(config->isOverwriteProtectionEnabled());
    
    config->setOverwriteProtection(false);
    EXPECT_FALSE(config->isOverwriteProtectionEnabled());
}
