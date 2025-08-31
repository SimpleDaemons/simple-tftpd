#include <gtest/gtest.h>
#include "simple_tftpd/logger.hpp"
#include "simple_tftpd/tftp_config.hpp"
#include "simple_tftpd/tftp_packet.hpp"

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
