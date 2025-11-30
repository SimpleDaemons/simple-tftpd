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

#include "simple-tftpd/utils/platform.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace simple_tftpd {

/**
 * @brief TFTP packet base class
 * 
 * Provides common functionality for all TFTP packet types.
 * TFTP packets are simple UDP packets with a specific format.
 */
class TftpPacket {
public:
    /**
     * @brief Constructor
     */
    TftpPacket();
    
    /**
     * @brief Constructor with opcode
     * @param opcode TFTP operation code
     */
    explicit TftpPacket(TftpOpcode opcode);
    
    /**
     * @brief Constructor from raw data
     * @param data Raw packet data
     * @param size Size of data
     */
    TftpPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Destructor
     */
    virtual ~TftpPacket();
    
    /**
     * @brief Get packet opcode
     * @return TFTP operation code
     */
    TftpOpcode getOpcode() const;
    
    /**
     * @brief Set packet opcode
     * @param opcode TFTP operation code
     */
    void setOpcode(TftpOpcode opcode);
    
    /**
     * @brief Get packet size
     * @return Size of packet in bytes
     */
    size_t getSize() const;
    
    /**
     * @brief Get raw packet data
     * @return Vector containing raw packet data
     */
    const std::vector<uint8_t>& getData() const;
    
    /**
     * @brief Set raw packet data
     * @param data Raw packet data
     */
    void setData(const std::vector<uint8_t>& data);
    
    /**
     * @brief Parse packet from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if parsed successfully, false otherwise
     */
    virtual bool parse(const uint8_t* data, size_t size);
    
    /**
     * @brief Serialize packet to raw data
     * @return Vector containing serialized packet data
     */
    virtual std::vector<uint8_t> serialize() const;
    
    /**
     * @brief Validate packet
     * @return true if packet is valid, false otherwise
     */
    virtual bool isValid() const;
    
    /**
     * @brief Get packet type string
     * @return String representation of packet type
     */
    virtual std::string getTypeString() const;

protected:
    TftpOpcode opcode_;
    std::vector<uint8_t> data_;
    
    /**
     * @brief Parse opcode from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if opcode parsed successfully, false otherwise
     */
    bool parseOpcode(const uint8_t* data, size_t size);
    
    /**
     * @brief Serialize opcode to raw data
     * @param data Vector to append serialized opcode to
     */
    void serializeOpcode(std::vector<uint8_t>& data) const;
};

/**
 * @brief TFTP Read/Write Request packet
 * 
 * Represents RRQ (Read Request) and WRQ (Write Request) packets.
 * Contains filename, mode, and optional parameters.
 */
class TftpRequestPacket : public TftpPacket {
public:
    /**
     * @brief Constructor
     * @param opcode Operation code (RRQ or WRQ)
     * @param filename Name of file to transfer
     * @param mode Transfer mode
     */
    TftpRequestPacket(TftpOpcode opcode, const std::string& filename, TftpMode mode);
    
    /**
     * @brief Constructor from raw data
     * @param data Raw packet data
     * @param size Size of data
     */
    TftpRequestPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Get filename
     * @return Filename
     */
    std::string getFilename() const;
    
    /**
     * @brief Set filename
     * @param filename Filename
     */
    void setFilename(const std::string& filename);
    
    /**
     * @brief Get transfer mode
     * @return Transfer mode
     */
    TftpMode getMode() const;
    
    /**
     * @brief Set transfer mode
     * @param mode Transfer mode
     */
    void setMode(TftpMode mode);
    
    /**
     * @brief Get options
     * @return TFTP options
     */
    TftpOptions getOptions() const;
    
    /**
     * @brief Set options
     * @param options TFTP options
     */
    void setOptions(const TftpOptions& options);
    
    /**
     * @brief Parse packet from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if parsed successfully, false otherwise
     */
    bool parse(const uint8_t* data, size_t size) override;
    
    /**
     * @brief Serialize packet to raw data
     * @return Vector containing serialized packet data
     */
    std::vector<uint8_t> serialize() const override;
    
    /**
     * @brief Validate packet
     * @return true if packet is valid, false otherwise
     */
    bool isValid() const override;
    
    /**
     * @brief Get packet type string
     * @return String representation of packet type
     */
    std::string getTypeString() const override;

private:
    std::string filename_;
    TftpMode mode_;
    TftpOptions options_;
    
    /**
     * @brief Parse options from raw data
     * @param data Raw packet data
     * @param offset Offset to start parsing from
     * @param size Size of data
     * @return true if options parsed successfully, false otherwise
     */
    bool parseOptions(const uint8_t* data, size_t offset, size_t size);
    
    /**
     * @brief Serialize options to raw data
     * @param data Vector to append serialized options to
     */
    void serializeOptions(std::vector<uint8_t>& data) const;
};

/**
 * @brief TFTP Data packet
 * 
 * Represents DATA packets containing file data blocks.
 */
class TftpDataPacket : public TftpPacket {
public:
    /**
     * @brief Constructor
     * @param block_number Block number
     * @param data File data
     */
    TftpDataPacket(uint16_t block_number, const std::vector<uint8_t>& data);
    
    /**
     * @brief Constructor from raw data
     * @param data Raw packet data
     * @param size Size of data
     */
    TftpDataPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Get block number
     * @return Block number
     */
    uint16_t getBlockNumber() const;
    
    /**
     * @brief Set block number
     * @param block_number Block number
     */
    void setBlockNumber(uint16_t block_number);
    
    /**
     * @brief Get file data
     * @return File data
     */
    const std::vector<uint8_t>& getFileData() const;
    
    /**
     * @brief Set file data
     * @param data File data
     */
    void setFileData(const std::vector<uint8_t>& data);
    
    /**
     * @brief Parse packet from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if parsed successfully, false otherwise
     */
    bool parse(const uint8_t* data, size_t size) override;
    
    /**
     * @brief Serialize packet to raw data
     * @return Vector containing serialized packet data
     */
    std::vector<uint8_t> serialize() const override;
    
    /**
     * @brief Validate packet
     * @return true if packet is valid, false otherwise
     */
    bool isValid() const override;
    
    /**
     * @brief Get packet type string
     * @return String representation of packet type
     */
    std::string getTypeString() const override;

private:
    uint16_t block_number_;
    std::vector<uint8_t> file_data_;
    
    /**
     * @brief Parse block number from raw data
     * @param data Raw packet data
     * @param offset Offset to start parsing from
     * @return true if block number parsed successfully, false otherwise
     */
    bool parseBlockNumber(const uint8_t* data, size_t offset);
    
    /**
     * @brief Serialize block number to raw data
     * @param data Vector to append serialized block number to
     */
    void serializeBlockNumber(std::vector<uint8_t>& data) const;
};

/**
 * @brief TFTP Acknowledgment packet
 * 
 * Represents ACK packets acknowledging data blocks.
 */
class TftpAckPacket : public TftpPacket {
public:
    /**
     * @brief Constructor
     * @param block_number Block number being acknowledged
     */
    explicit TftpAckPacket(uint16_t block_number);
    
    /**
     * @brief Constructor from raw data
     * @param data Raw packet data
     * @param size Size of data
     */
    TftpAckPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Get block number
     * @return Block number
     */
    uint16_t getBlockNumber() const;
    
    /**
     * @brief Set block number
     * @param block_number Block number
     */
    void setBlockNumber(uint16_t block_number);
    
    /**
     * @brief Parse packet from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if parsed successfully, false otherwise
     */
    bool parse(const uint8_t* data, size_t size) override;
    
    /**
     * @brief Serialize packet to raw data
     * @return Vector containing serialized packet data
     */
    std::vector<uint8_t> serialize() const override;
    
    /**
     * @brief Validate packet
     * @return true if packet is valid, false otherwise
     */
    bool isValid() const override;
    
    /**
     * @brief Get packet type string
     * @return String representation of packet type
     */
    std::string getTypeString() const override;

private:
    uint16_t block_number_;
    
    /**
     * @brief Parse block number from raw data
     * @param data Raw packet data
     * @param offset Offset to start parsing from
     * @return true if block number parsed successfully, false otherwise
     */
    bool parseBlockNumber(const uint8_t* data, size_t offset);
    
    /**
     * @brief Serialize block number to raw data
     * @param data Vector to append serialized block number to
     */
    void serializeBlockNumber(std::vector<uint8_t>& data) const;
};

/**
 * @brief TFTP Error packet
 * 
 * Represents ERROR packets for error conditions.
 */
class TftpErrorPacket : public TftpPacket {
public:
    /**
     * @brief Constructor
     * @param error_code Error code
     * @param error_message Error message
     */
    TftpErrorPacket(TftpError error_code, const std::string& error_message);
    
    /**
     * @brief Constructor from raw data
     * @param data Raw packet data
     * @param size Size of data
     */
    TftpErrorPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Get error code
     * @return Error code
     */
    TftpError getErrorCode() const;
    
    /**
     * @brief Set error code
     * @param error_code Error code
     */
    void setErrorCode(TftpError error_code);
    
    /**
     * @brief Get error message
     * @return Error message
     */
    std::string getErrorMessage() const;
    
    /**
     * @brief Set error message
     * @param error_message Error message
     */
    void setErrorMessage(const std::string& error_message);
    
    /**
     * @brief Parse packet from raw data
     * @param data Raw packet data
     * @param size Size of data
     * @return true if parsed successfully, false otherwise
     */
    bool parse(const uint8_t* data, size_t size) override;
    
    /**
     * @brief Serialize packet to raw data
     * @return Vector containing serialized packet data
     */
    std::vector<uint8_t> serialize() const override;
    
    /**
     * @brief Validate packet
     * @return true if packet is valid, false otherwise
     */
    bool isValid() const override;
    
    /**
     * @brief Get packet type string
     * @return String representation of packet type
     */
    std::string getTypeString() const override;

private:
    TftpError error_code_;
    std::string error_message_;
    
    /**
     * @brief Parse error code from raw data
     * @param data Raw packet data
     * @param offset Offset to start parsing from
     * @return true if error code parsed successfully, false otherwise
     */
    bool parseErrorCode(const uint8_t* data, size_t offset);
    
    /**
     * @brief Serialize error code to raw data
     * @param data Vector to append serialized error code to
     */
    void serializeErrorCode(std::vector<uint8_t>& data) const;
};

} // namespace simple_tftpd
