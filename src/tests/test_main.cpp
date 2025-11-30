#include <gtest/gtest.h>
#include "simple-tftpd/utils/logger.hpp"
#include "simple-tftpd/config.hpp"
#include "simple-tftpd/packet.hpp"

// Basic test to ensure the test framework works
TEST(BasicTest, TestFramework) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

// Test Logger class
TEST(LoggerTest, BasicFunctionality) {
    simple_tftpd::Logger logger;
    
    // Test default log level
    EXPECT_EQ(logger.getLevel(), simple_tftpd::LogLevel::INFO);
    
    // Test setting log level
    logger.setLevel(simple_tftpd::LogLevel::DEBUG);
    EXPECT_EQ(logger.getLevel(), simple_tftpd::LogLevel::DEBUG);
    
    // Test console logging
    logger.setConsoleOutput(true);
}

// Test TftpConfig class
TEST(TftpConfigTest, DefaultValues) {
    simple_tftpd::TftpConfig config;
    
    // Test default network settings
    EXPECT_EQ(config.getListenAddress(), "0.0.0.0");
    EXPECT_EQ(config.getListenPort(), 69);
    EXPECT_TRUE(config.isIpv6Enabled());
    
    // Test default filesystem settings
    EXPECT_EQ(config.getRootDirectory(), "/var/tftp");
    
    // Test default security settings
    EXPECT_EQ(config.isReadEnabled(), true);
    EXPECT_EQ(config.isWriteEnabled(), false);
    EXPECT_EQ(config.getMaxFileSize(), 104857600);
}

TEST(TftpConfigTest, MaxRetriesConfiguration) {
    simple_tftpd::TftpConfig config;
    EXPECT_EQ(config.getMaxRetries(), 5);
    
    config.setMaxRetries(0); // should clamp to at least 1
    EXPECT_EQ(config.getMaxRetries(), 1);
    
    config.setMaxRetries(8);
    EXPECT_EQ(config.getMaxRetries(), 8);
    
    const std::string json = R"({
        "performance": {
            "max_retries": 11,
            "timeout": 9
        }
    })";
    
    EXPECT_TRUE(config.loadFromJson(json));
    EXPECT_EQ(config.getMaxRetries(), 11);
    EXPECT_EQ(config.getTimeout(), 9);
}

TEST(TftpConfigTest, AllowedExtensions) {
    simple_tftpd::TftpConfig config;
    config.setAllowedExtensions({"bin", ".IMG"});
    
    EXPECT_TRUE(config.isExtensionAllowed("bin"));
    EXPECT_TRUE(config.isExtensionAllowed("IMG"));
    EXPECT_FALSE(config.isExtensionAllowed("txt"));
}

TEST(TftpConfigTest, AllowedClients) {
    simple_tftpd::TftpConfig config;
    EXPECT_TRUE(config.isClientAllowed("10.0.0.1")); // default allow
    
    config.setAllowedClients({"192.168.1.5", "fe80::1"});
    EXPECT_TRUE(config.isClientAllowed("192.168.1.5"));
    EXPECT_FALSE(config.isClientAllowed("10.0.0.1"));
    
    config.setAllowedClients({"*"});
    EXPECT_TRUE(config.isClientAllowed("203.0.113.7"));
}

// Test TftpPacket class
TEST(TftpPacketTest, BasicPacket) {
    // Create a basic packet
    simple_tftpd::TftpPacket packet(simple_tftpd::TftpOpcode::RRQ);
    
    // Test basic properties
    EXPECT_EQ(packet.getOpcode(), simple_tftpd::TftpOpcode::RRQ);
    EXPECT_TRUE(packet.isValid());
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
