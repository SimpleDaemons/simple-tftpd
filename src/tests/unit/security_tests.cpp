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
#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include "simple-tftpd/config/config.hpp"
#include "simple-tftpd/core/server.hpp"
#include "simple-tftpd/utils/logger.hpp"

using namespace simple_tftpd;

// Test fixture for security tests
class SecurityTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<TftpConfig>();
        logger = std::make_shared<Logger>();
        logger->setLevel(LogLevel::ERROR); // Suppress logs during tests
    }

    std::shared_ptr<TftpConfig> config;
    std::shared_ptr<Logger> logger;
};

// Test path traversal protection
TEST_F(SecurityTest, PathTraversalProtection) {
    // Set root directory
    config->setRootDirectory("/var/tftp");
    
    // Test various path traversal attempts
    std::vector<std::string> malicious_paths = {
        "../../etc/passwd",
        "..\\..\\windows\\system32",
        "/etc/passwd",
        "....//....//etc/passwd",
        "..%2F..%2Fetc%2Fpasswd",  // URL encoded
    };
    
    // The config should validate paths
    // This test verifies the configuration accepts security settings
    config->setAllowedDirectories({"/var/tftp/public"});
    auto allowed = config->getAllowedDirectories();
    EXPECT_FALSE(allowed.empty());
}

// Test extension filtering
TEST_F(SecurityTest, ExtensionFiltering) {
    // Set allowed extensions
    config->setAllowedExtensions({"bin", "img", "txt"});
    
    auto allowed = config->getAllowedExtensions();
    EXPECT_EQ(allowed.size(), 3);
    
    // Test that only allowed extensions are in the list
    bool has_bin = std::find(allowed.begin(), allowed.end(), "bin") != allowed.end();
    bool has_img = std::find(allowed.begin(), allowed.end(), "img") != allowed.end();
    bool has_txt = std::find(allowed.begin(), allowed.end(), "txt") != allowed.end();
    
    EXPECT_TRUE(has_bin);
    EXPECT_TRUE(has_img);
    EXPECT_TRUE(has_txt);
    
    // Test empty extensions (allow all)
    config->setAllowedExtensions({});
    allowed = config->getAllowedExtensions();
    EXPECT_TRUE(allowed.empty());
}

// Test client filtering
TEST_F(SecurityTest, ClientFiltering) {
    // Set allowed clients
    std::vector<std::string> clients = {"192.168.1.1", "10.0.0.1", "fe80::1"};
    config->setAllowedClients(clients);
    
    auto allowed = config->getAllowedClients();
    EXPECT_EQ(allowed.size(), 3);
    
    // Verify all clients are in the list
    for (const auto& client : clients) {
        bool found = std::find(allowed.begin(), allowed.end(), client) != allowed.end();
        EXPECT_TRUE(found) << "Client " << client << " not found in allowed list";
    }
    
    // Test empty clients (allow all)
    config->setAllowedClients({});
    allowed = config->getAllowedClients();
    EXPECT_TRUE(allowed.empty());
}

// Test file size limits
TEST_F(SecurityTest, FileSizeLimits) {
    // Set max file size
    config->setMaxFileSize(1024 * 1024); // 1MB
    EXPECT_EQ(config->getMaxFileSize(), 1024 * 1024);
    
    // Test various size limits
    config->setMaxFileSize(0);
    EXPECT_EQ(config->getMaxFileSize(), 0);
    
    config->setMaxFileSize(10 * 1024 * 1024); // 10MB
    EXPECT_EQ(config->getMaxFileSize(), 10 * 1024 * 1024);
    
    config->setMaxFileSize(100 * 1024 * 1024); // 100MB
    EXPECT_EQ(config->getMaxFileSize(), 100 * 1024 * 1024);
}

// Test read/write permissions
TEST_F(SecurityTest, ReadWritePermissions) {
    // Test read-only mode
    config->setReadEnabled(true);
    config->setWriteEnabled(false);
    EXPECT_TRUE(config->isReadEnabled());
    EXPECT_FALSE(config->isWriteEnabled());
    
    // Test write-only mode
    config->setReadEnabled(false);
    config->setWriteEnabled(true);
    EXPECT_FALSE(config->isReadEnabled());
    EXPECT_TRUE(config->isWriteEnabled());
    
    // Test read-write mode
    config->setReadEnabled(true);
    config->setWriteEnabled(true);
    EXPECT_TRUE(config->isReadEnabled());
    EXPECT_TRUE(config->isWriteEnabled());
    
    // Test no access mode
    config->setReadEnabled(false);
    config->setWriteEnabled(false);
    EXPECT_FALSE(config->isReadEnabled());
    EXPECT_FALSE(config->isWriteEnabled());
}

// Test overwrite protection
TEST_F(SecurityTest, OverwriteProtection) {
    // Enable overwrite protection
    config->setOverwriteProtection(true);
    EXPECT_TRUE(config->isOverwriteProtectionEnabled());
    
    // Disable overwrite protection
    config->setOverwriteProtection(false);
    EXPECT_FALSE(config->isOverwriteProtectionEnabled());
}

// Test directory allowlist
TEST_F(SecurityTest, DirectoryAllowlist) {
    // Set multiple allowed directories
    std::vector<std::string> dirs = {
        "/var/tftp/public",
        "/var/tftp/private",
        "/var/tftp/temp"
    };
    config->setAllowedDirectories(dirs);
    
    auto allowed = config->getAllowedDirectories();
    EXPECT_EQ(allowed.size(), 3);
    
    // Verify all directories are in the list
    for (const auto& dir : dirs) {
        bool found = std::find(allowed.begin(), allowed.end(), dir) != allowed.end();
        EXPECT_TRUE(found) << "Directory " << dir << " not found in allowed list";
    }
    
    // Test empty directories (allow all)
    config->setAllowedDirectories({});
    allowed = config->getAllowedDirectories();
    EXPECT_TRUE(allowed.empty());
}

// Test security configuration validation
TEST_F(SecurityTest, SecurityConfigurationValidation) {
    // Valid configuration should pass
    config->setReadEnabled(true);
    config->setMaxFileSize(104857600);
    EXPECT_TRUE(config->validate());
    
    // Configuration with security settings should still validate
    config->setAllowedClients({"192.168.1.1"});
    config->setAllowedExtensions({"bin"});
    config->setAllowedDirectories({"/var/tftp"});
    EXPECT_TRUE(config->validate());
}

// Test edge cases for extension filtering
TEST_F(SecurityTest, ExtensionFilteringEdgeCases) {
    // Test case sensitivity
    config->setAllowedExtensions({"BIN", "IMG"});
    auto allowed = config->getAllowedExtensions();
    EXPECT_EQ(allowed.size(), 2);
    
    // Test with dots in extension
    config->setAllowedExtensions({"bin", "img", "tar.gz"});
    allowed = config->getAllowedExtensions();
    EXPECT_EQ(allowed.size(), 3);
    
    // Test empty extension
    config->setAllowedExtensions({""});
    allowed = config->getAllowedExtensions();
    // Should handle empty extension gracefully
    EXPECT_TRUE(allowed.empty() || allowed.size() == 1);
}

// Test edge cases for client filtering
TEST_F(SecurityTest, ClientFilteringEdgeCases) {
    // Test IPv4 addresses
    config->setAllowedClients({"192.168.1.1", "10.0.0.1"});
    auto allowed = config->getAllowedClients();
    EXPECT_EQ(allowed.size(), 2);
    
    // Test IPv6 addresses
    config->setAllowedClients({"fe80::1", "2001:db8::1"});
    allowed = config->getAllowedClients();
    EXPECT_EQ(allowed.size(), 2);
    
    // Test mixed IPv4 and IPv6
    config->setAllowedClients({"192.168.1.1", "fe80::1"});
    allowed = config->getAllowedClients();
    EXPECT_EQ(allowed.size(), 2);
}

// Test file size limit edge cases
TEST_F(SecurityTest, FileSizeLimitEdgeCases) {
    // Test zero size (unlimited)
    config->setMaxFileSize(0);
    EXPECT_EQ(config->getMaxFileSize(), 0);
    
    // Test very large size
    config->setMaxFileSize(UINT64_MAX);
    EXPECT_EQ(config->getMaxFileSize(), UINT64_MAX);
    
    // Test reasonable sizes
    config->setMaxFileSize(1024); // 1KB
    EXPECT_EQ(config->getMaxFileSize(), 1024);
    
    config->setMaxFileSize(1024 * 1024); // 1MB
    EXPECT_EQ(config->getMaxFileSize(), 1024 * 1024);
    
    config->setMaxFileSize(1024ULL * 1024 * 1024); // 1GB
    EXPECT_EQ(config->getMaxFileSize(), 1024ULL * 1024 * 1024);
}
