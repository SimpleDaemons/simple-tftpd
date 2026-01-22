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
#include "simple-tftpd/core/tftp/packet.hpp"
#include <vector>
#include <cstring>

using namespace simple_tftpd;

// Test TftpPacket base class
class TftpPacketTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpPacketTest, BasicPacketCreation) {
    TftpPacket packet(TftpOpcode::RRQ);
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::RRQ);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpPacketTest, PacketOpcodeSetGet) {
    TftpPacket packet;
    packet.setOpcode(TftpOpcode::WRQ);
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::WRQ);
}

TEST_F(TftpPacketTest, InvalidPacketFromEmptyData) {
    uint8_t empty_data[1] = {0}; // Use array with at least one element
    TftpPacket packet(empty_data, 0);
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpPacketTest, InvalidPacketFromTooSmallData) {
    uint8_t small_data[] = {0x00}; // Only 1 byte, need at least 2 for opcode
    TftpPacket packet(small_data, 1);
    EXPECT_FALSE(packet.isValid());
}

// Test TftpRequestPacket
class TftpRequestPacketTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpRequestPacketTest, RRQPacketCreation) {
    TftpRequestPacket packet(TftpOpcode::RRQ, "test.txt", TftpMode::OCTET);
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::RRQ);
    EXPECT_EQ(packet.getFilename(), "test.txt");
    EXPECT_EQ(packet.getMode(), TftpMode::OCTET);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpRequestPacketTest, WRQPacketCreation) {
    TftpRequestPacket packet(TftpOpcode::WRQ, "upload.bin", TftpMode::OCTET);
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::WRQ);
    EXPECT_EQ(packet.getFilename(), "upload.bin");
    EXPECT_EQ(packet.getMode(), TftpMode::OCTET);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpRequestPacketTest, PacketSerialization) {
    TftpRequestPacket packet(TftpOpcode::RRQ, "test.txt", TftpMode::OCTET);
    std::vector<uint8_t> serialized = packet.serialize();
    
    EXPECT_GT(serialized.size(), 0);
    // Should contain opcode (2 bytes) + filename + null + mode + null
    EXPECT_GE(serialized.size(), 2 + 8 + 1 + 5 + 1); // "test.txt" + "octet"
}

TEST_F(TftpRequestPacketTest, PacketDeserialization) {
    TftpRequestPacket original(TftpOpcode::RRQ, "test.txt", TftpMode::OCTET);
    std::vector<uint8_t> data = original.serialize();
    
    TftpRequestPacket parsed(data.data(), data.size());
    EXPECT_EQ(parsed.getOpcode(), TftpOpcode::RRQ);
    EXPECT_EQ(parsed.getFilename(), "test.txt");
    EXPECT_EQ(parsed.getMode(), TftpMode::OCTET);
    EXPECT_TRUE(parsed.isValid());
}

TEST_F(TftpRequestPacketTest, PacketWithOptions) {
    TftpRequestPacket packet(TftpOpcode::RRQ, "test.txt", TftpMode::OCTET);
    TftpOptions options;
    options.has_blksize = true;
    options.blksize = 1024;
    options.has_timeout = true;
    options.timeout = 10;
    packet.setOptions(options);
    
    TftpOptions retrieved = packet.getOptions();
    EXPECT_TRUE(retrieved.has_blksize);
    EXPECT_EQ(retrieved.blksize, 1024);
    EXPECT_TRUE(retrieved.has_timeout);
    EXPECT_EQ(retrieved.timeout, 10);
}

TEST_F(TftpRequestPacketTest, InvalidPacketFromCorruptedData) {
    uint8_t corrupted[] = {0x00, 0x01, 0xFF, 0xFF}; // Invalid format
    TftpRequestPacket packet(corrupted, sizeof(corrupted));
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpRequestPacketTest, LongFilename) {
    std::string long_filename(200, 'a');
    long_filename += ".txt";
    TftpRequestPacket packet(TftpOpcode::RRQ, long_filename, TftpMode::OCTET);
    
    EXPECT_EQ(packet.getFilename(), long_filename);
    EXPECT_TRUE(packet.isValid());
}

// Test TftpDataPacket
class TftpDataPacketTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpDataPacketTest, DataPacketCreation) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    TftpDataPacket packet(1, data);
    
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::DATA);
    EXPECT_EQ(packet.getBlockNumber(), 1);
    EXPECT_EQ(packet.getFileData(), data);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpDataPacketTest, DataPacketSerialization) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    TftpDataPacket packet(1, data);
    std::vector<uint8_t> serialized = packet.serialize();
    
    EXPECT_GT(serialized.size(), 0);
    // Should contain opcode (2 bytes) + block number (2 bytes) + data
    EXPECT_EQ(serialized.size(), 2 + 2 + data.size());
}

TEST_F(TftpDataPacketTest, DataPacketDeserialization) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    TftpDataPacket original(1, data);
    std::vector<uint8_t> serialized = original.serialize();
    
    TftpDataPacket parsed(serialized.data(), serialized.size());
    EXPECT_EQ(parsed.getOpcode(), TftpOpcode::DATA);
    EXPECT_EQ(parsed.getBlockNumber(), 1);
    EXPECT_EQ(parsed.getFileData(), data);
    EXPECT_TRUE(parsed.isValid());
}

TEST_F(TftpDataPacketTest, EmptyDataPacket) {
    std::vector<uint8_t> empty_data;
    TftpDataPacket packet(1, empty_data);
    
    EXPECT_EQ(packet.getBlockNumber(), 1);
    EXPECT_EQ(packet.getFileData().size(), 0);
    EXPECT_TRUE(packet.isValid()); // Empty data packet is valid (last packet)
}

TEST_F(TftpDataPacketTest, LargeDataPacket) {
    std::vector<uint8_t> large_data(512, 0x42); // Maximum block size
    TftpDataPacket packet(100, large_data);
    
    EXPECT_EQ(packet.getBlockNumber(), 100);
    EXPECT_EQ(packet.getFileData().size(), 512);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpDataPacketTest, BlockNumberWrap) {
    std::vector<uint8_t> data = {0x01, 0x02};
    TftpDataPacket packet(65535, data); // Max uint16_t
    
    EXPECT_EQ(packet.getBlockNumber(), 65535);
    EXPECT_TRUE(packet.isValid());
}

// Test TftpAckPacket
class TftpAckPacketTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpAckPacketTest, AckPacketCreation) {
    TftpAckPacket packet(1);
    
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::ACK);
    EXPECT_EQ(packet.getBlockNumber(), 1);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpAckPacketTest, AckPacketSerialization) {
    TftpAckPacket packet(1);
    std::vector<uint8_t> serialized = packet.serialize();
    
    EXPECT_EQ(serialized.size(), 4); // Opcode (2 bytes) + block number (2 bytes)
}

TEST_F(TftpAckPacketTest, AckPacketDeserialization) {
    TftpAckPacket original(1);
    std::vector<uint8_t> serialized = original.serialize();
    
    TftpAckPacket parsed(serialized.data(), serialized.size());
    EXPECT_EQ(parsed.getOpcode(), TftpOpcode::ACK);
    EXPECT_EQ(parsed.getBlockNumber(), 1);
    EXPECT_TRUE(parsed.isValid());
}

TEST_F(TftpAckPacketTest, AckBlockNumberZero) {
    TftpAckPacket packet(0);
    EXPECT_EQ(packet.getBlockNumber(), 0);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpAckPacketTest, AckBlockNumberMax) {
    TftpAckPacket packet(65535);
    EXPECT_EQ(packet.getBlockNumber(), 65535);
    EXPECT_TRUE(packet.isValid());
}

// Test TftpErrorPacket
class TftpErrorPacketTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpErrorPacketTest, ErrorPacketCreation) {
    TftpErrorPacket packet(TftpError::FILE_NOT_FOUND, "File not found");
    
    EXPECT_EQ(packet.getOpcode(), TftpOpcode::ERROR);
    EXPECT_EQ(packet.getErrorCode(), TftpError::FILE_NOT_FOUND);
    EXPECT_EQ(packet.getErrorMessage(), "File not found");
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpErrorPacketTest, ErrorPacketSerialization) {
    TftpErrorPacket packet(TftpError::ACCESS_VIOLATION, "Access denied");
    std::vector<uint8_t> serialized = packet.serialize();
    
    EXPECT_GT(serialized.size(), 0);
    // Should contain opcode (2 bytes) + error code (2 bytes) + message + null
    EXPECT_GE(serialized.size(), 2 + 2 + 12 + 1); // "Access denied" + null
}

TEST_F(TftpErrorPacketTest, ErrorPacketDeserialization) {
    TftpErrorPacket original(TftpError::DISK_FULL, "Disk full");
    std::vector<uint8_t> serialized = original.serialize();
    
    TftpErrorPacket parsed(serialized.data(), serialized.size());
    EXPECT_EQ(parsed.getOpcode(), TftpOpcode::ERROR);
    EXPECT_EQ(parsed.getErrorCode(), TftpError::DISK_FULL);
    EXPECT_EQ(parsed.getErrorMessage(), "Disk full");
    EXPECT_TRUE(parsed.isValid());
}

TEST_F(TftpErrorPacketTest, AllErrorCodes) {
    std::vector<TftpError> error_codes = {
        TftpError::FILE_NOT_FOUND,
        TftpError::ACCESS_VIOLATION,
        TftpError::DISK_FULL,
        TftpError::ILLEGAL_OPERATION,
        TftpError::UNKNOWN_TRANSFER_ID,
        TftpError::FILE_EXISTS,
        TftpError::NO_SUCH_USER
    };
    
    for (auto error_code : error_codes) {
        TftpErrorPacket packet(error_code, "Test error");
        EXPECT_EQ(packet.getErrorCode(), error_code);
        EXPECT_TRUE(packet.isValid());
    }
}

TEST_F(TftpErrorPacketTest, EmptyErrorMessage) {
    TftpErrorPacket packet(TftpError::NETWORK_ERROR, "");
    EXPECT_EQ(packet.getErrorCode(), TftpError::NETWORK_ERROR);
    EXPECT_EQ(packet.getErrorMessage(), "");
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpErrorPacketTest, LongErrorMessage) {
    std::string long_message(200, 'x');
    TftpErrorPacket packet(TftpError::TIMEOUT, long_message);
    EXPECT_EQ(packet.getErrorMessage(), long_message);
    EXPECT_TRUE(packet.isValid());
}

// Test packet parsing edge cases
class TftpPacketEdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TftpPacketEdgeCasesTest, InvalidOpcode) {
    uint8_t invalid_opcode[] = {0x00, 0x00}; // Opcode 0 is invalid
    TftpPacket packet(invalid_opcode, sizeof(invalid_opcode));
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpPacketEdgeCasesTest, MalformedRequestPacket) {
    // Missing null terminators
    uint8_t malformed[] = {0x00, 0x01, 't', 'e', 's', 't'}; // No null after filename
    TftpRequestPacket packet(malformed, sizeof(malformed));
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpPacketEdgeCasesTest, MalformedDataPacket) {
    // Too small for data packet (need at least 4 bytes: opcode + block number)
    uint8_t malformed[] = {0x00, 0x03, 0x00}; // Only 3 bytes
    TftpDataPacket packet(malformed, sizeof(malformed));
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpPacketEdgeCasesTest, MalformedAckPacket) {
    // Too small for ACK packet (need 4 bytes)
    uint8_t malformed[] = {0x00, 0x04, 0x00}; // Only 3 bytes
    TftpAckPacket packet(malformed, sizeof(malformed));
    EXPECT_FALSE(packet.isValid());
}

TEST_F(TftpPacketEdgeCasesTest, RoundTripSerialization) {
    // Test that serialization and deserialization preserve data
    TftpRequestPacket original(TftpOpcode::WRQ, "test.bin", TftpMode::OCTET);
    std::vector<uint8_t> serialized = original.serialize();
    
    TftpRequestPacket parsed(serialized.data(), serialized.size());
    EXPECT_EQ(parsed.getOpcode(), original.getOpcode());
    EXPECT_EQ(parsed.getFilename(), original.getFilename());
    EXPECT_EQ(parsed.getMode(), original.getMode());
}
