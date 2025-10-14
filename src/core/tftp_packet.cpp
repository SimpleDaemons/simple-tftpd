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

#include "simple_tftpd/tftp_packet.hpp"
#include <cstring>

namespace simple_tftpd {

// TftpPacket base class
TftpPacket::TftpPacket() : opcode_(TftpOpcode::ERROR) {}

TftpPacket::TftpPacket(TftpOpcode opcode) : opcode_(opcode) {}

TftpPacket::TftpPacket(const uint8_t* data, size_t size) : opcode_(TftpOpcode::ERROR) {
    parse(data, size);
}

TftpPacket::~TftpPacket() = default;

TftpOpcode TftpPacket::getOpcode() const {
    return opcode_;
}

void TftpPacket::setOpcode(TftpOpcode opcode) {
    opcode_ = opcode;
}

size_t TftpPacket::getSize() const {
    return data_.size();
}

const std::vector<uint8_t>& TftpPacket::getData() const {
    return data_;
}

void TftpPacket::setData(const std::vector<uint8_t>& data) {
    data_ = data;
}

bool TftpPacket::parse(const uint8_t* data, size_t size) {
    if (!data || size < 2) {
        return false;
    }
    
    return parseOpcode(data, size);
}

std::vector<uint8_t> TftpPacket::serialize() const {
    std::vector<uint8_t> result;
    serializeOpcode(result);
    return result;
}

bool TftpPacket::isValid() const {
    return opcode_ != TftpOpcode::ERROR;
}

std::string TftpPacket::getTypeString() const {
    return "TFTP_PACKET";
}

bool TftpPacket::parseOpcode(const uint8_t* data, size_t size) {
    if (size < 2) {
        return false;
    }
    
    uint16_t opcode_value = (data[0] << 8) | data[1];
    opcode_ = static_cast<TftpOpcode>(opcode_value);
    
    return true;
}

void TftpPacket::serializeOpcode(std::vector<uint8_t>& data) const {
    uint16_t opcode_value = static_cast<uint16_t>(opcode_);
    data.push_back((opcode_value >> 8) & 0xFF);
    data.push_back(opcode_value & 0xFF);
}

// TftpRequestPacket implementation
TftpRequestPacket::TftpRequestPacket(TftpOpcode opcode, const std::string& filename, TftpMode mode)
    : TftpPacket(opcode), filename_(filename), mode_(mode) {}

TftpRequestPacket::TftpRequestPacket(const uint8_t* data, size_t size)
    : TftpPacket(data, size) {
    parse(data, size);
}

std::string TftpRequestPacket::getFilename() const {
    return filename_;
}

void TftpRequestPacket::setFilename(const std::string& filename) {
    filename_ = filename;
}

TftpMode TftpRequestPacket::getMode() const {
    return mode_;
}

void TftpRequestPacket::setMode(TftpMode mode) {
    mode_ = mode;
}

TftpOptions TftpRequestPacket::getOptions() const {
    return options_;
}

void TftpRequestPacket::setOptions(const TftpOptions& options) {
    options_ = options;
}

bool TftpRequestPacket::parse(const uint8_t* data, size_t size) {
    if (!TftpPacket::parse(data, size)) {
        return false;
    }
    
    if (size < 4) { // Minimum: opcode(2) + filename(1) + mode(1)
        return false;
    }
    
    size_t offset = 2; // Skip opcode
    
    // Parse filename (null-terminated string)
    std::string filename;
    while (offset < size && data[offset] != 0) {
        filename += static_cast<char>(data[offset]);
        offset++;
    }
    
    if (offset >= size || filename.empty()) {
        return false;
    }
    
    filename_ = filename;
    offset++; // Skip null terminator
    
    // Parse mode (null-terminated string)
    std::string mode_str;
    while (offset < size && data[offset] != 0) {
        mode_str += static_cast<char>(data[offset]);
        offset++;
    }
    
    if (offset >= size || mode_str.empty()) {
        return false;
    }
    
    // Convert mode string to enum
    if (mode_str == "netascii") {
        mode_ = TftpMode::NETASCII;
    } else if (mode_str == "octet") {
        mode_ = TftpMode::OCTET;
    } else if (mode_str == "mail") {
        mode_ = TftpMode::MAIL;
    } else {
        return false; // Invalid mode
    }
    
    // Parse options if present
    if (offset < size) {
        parseOptions(data, offset, size);
    }
    
    return true;
}

std::vector<uint8_t> TftpRequestPacket::serialize() const {
    std::vector<uint8_t> result = TftpPacket::serialize();
    
    // Add filename
    result.insert(result.end(), filename_.begin(), filename_.end());
    result.push_back(0); // null terminator
    
    // Add mode string
    std::string mode_str;
    switch (mode_) {
        case TftpMode::NETASCII:
            mode_str = "netascii";
            break;
        case TftpMode::OCTET:
            mode_str = "octet";
            break;
        case TftpMode::MAIL:
            mode_str = "mail";
            break;
    }
    result.insert(result.end(), mode_str.begin(), mode_str.end());
    result.push_back(0); // null terminator
    
    // Add options if present
    serializeOptions(result);
    
    return result;
}

bool TftpRequestPacket::isValid() const {
    return TftpPacket::isValid() && !filename_.empty();
}

std::string TftpRequestPacket::getTypeString() const {
    return "TFTP_REQUEST";
}

bool TftpRequestPacket::parseOptions(const uint8_t* data, size_t offset, size_t size) {
    // Reset options to defaults
    options_ = TftpOptions{};
    
    while (offset < size) {
        // Parse option name (null-terminated string)
        std::string option_name;
        while (offset < size && data[offset] != 0) {
            option_name += static_cast<char>(data[offset]);
            offset++;
        }
        
        if (offset >= size) {
            break; // No more data
        }
        
        offset++; // Skip null terminator
        
        // Parse option value (null-terminated string)
        std::string option_value;
        while (offset < size && data[offset] != 0) {
            option_value += static_cast<char>(data[offset]);
            offset++;
        }
        
        if (offset >= size) {
            break; // No more data
        }
        
        offset++; // Skip null terminator
        
        // Process known options
        if (option_name == "blksize") {
            try {
                options_.blksize = static_cast<uint16_t>(std::stoi(option_value));
            } catch (...) {
                // Invalid blksize value, ignore
            }
        } else if (option_name == "timeout") {
            try {
                options_.timeout = static_cast<uint16_t>(std::stoi(option_value));
            } catch (...) {
                // Invalid timeout value, ignore
            }
        } else if (option_name == "tsize") {
            try {
                options_.tsize = static_cast<uint16_t>(std::stoi(option_value));
            } catch (...) {
                // Invalid tsize value, ignore
            }
        } else if (option_name == "windowsize") {
            try {
                options_.windowsize = static_cast<uint16_t>(std::stoi(option_value));
            } catch (...) {
                // Invalid windowsize value, ignore
            }
        }
        // Unknown options are ignored
    }
    
    return true;
}

void TftpRequestPacket::serializeOptions(std::vector<uint8_t>& data) const {
    // Only serialize non-default options
    if (options_.blksize != 512) {
        std::string blksize_str = std::to_string(options_.blksize);
        data.insert(data.end(), "blksize", "blksize" + 7);
        data.push_back(0);
        data.insert(data.end(), blksize_str.begin(), blksize_str.end());
        data.push_back(0);
    }
    
    if (options_.timeout != 5) {
        std::string timeout_str = std::to_string(options_.timeout);
        data.insert(data.end(), "timeout", "timeout" + 7);
        data.push_back(0);
        data.insert(data.end(), timeout_str.begin(), timeout_str.end());
        data.push_back(0);
    }
    
    if (options_.tsize != 0) {
        std::string tsize_str = std::to_string(options_.tsize);
        data.insert(data.end(), "tsize", "tsize" + 5);
        data.push_back(0);
        data.insert(data.end(), tsize_str.begin(), tsize_str.end());
        data.push_back(0);
    }
    
    if (options_.windowsize != 1) {
        std::string windowsize_str = std::to_string(options_.windowsize);
        data.insert(data.end(), "windowsize", "windowsize" + 10);
        data.push_back(0);
        data.insert(data.end(), windowsize_str.begin(), windowsize_str.end());
        data.push_back(0);
    }
}

// TftpDataPacket implementation
TftpDataPacket::TftpDataPacket(uint16_t block_number, const std::vector<uint8_t>& data)
    : TftpPacket(TftpOpcode::DATA), block_number_(block_number), file_data_(data) {}

TftpDataPacket::TftpDataPacket(const uint8_t* data, size_t size)
    : TftpPacket(data, size) {
    parse(data, size);
}

uint16_t TftpDataPacket::getBlockNumber() const {
    return block_number_;
}

void TftpDataPacket::setBlockNumber(uint16_t block_number) {
    block_number_ = block_number;
}

const std::vector<uint8_t>& TftpDataPacket::getFileData() const {
    return file_data_;
}

void TftpDataPacket::setFileData(const std::vector<uint8_t>& data) {
    file_data_ = data;
}

bool TftpDataPacket::parse(const uint8_t* data, size_t size) {
    if (!TftpPacket::parse(data, size)) {
        return false;
    }
    
    if (size < 4) {
        return false;
    }
    
    return parseBlockNumber(data, 2);
}

std::vector<uint8_t> TftpDataPacket::serialize() const {
    std::vector<uint8_t> result = TftpPacket::serialize();
    serializeBlockNumber(result);
    result.insert(result.end(), file_data_.begin(), file_data_.end());
    return result;
}

bool TftpDataPacket::isValid() const {
    return TftpPacket::isValid() && opcode_ == TftpOpcode::DATA;
}

std::string TftpDataPacket::getTypeString() const {
    return "TFTP_DATA";
}

bool TftpDataPacket::parseBlockNumber(const uint8_t* data, size_t offset) {
    // Note: This is a basic implementation - in a real implementation,
    // we would need to pass the total data size as a parameter
    // For now, just assume we have enough data
    block_number_ = (data[offset] << 8) | data[offset + 1];
    return true;
}

void TftpDataPacket::serializeBlockNumber(std::vector<uint8_t>& data) const {
    data.push_back((block_number_ >> 8) & 0xFF);
    data.push_back(block_number_ & 0xFF);
}

// TftpAckPacket implementation
TftpAckPacket::TftpAckPacket(uint16_t block_number)
    : TftpPacket(TftpOpcode::ACK), block_number_(block_number) {}

TftpAckPacket::TftpAckPacket(const uint8_t* data, size_t size)
    : TftpPacket(data, size) {
    parse(data, size);
}

uint16_t TftpAckPacket::getBlockNumber() const {
    return block_number_;
}

void TftpAckPacket::setBlockNumber(uint16_t block_number) {
    block_number_ = block_number;
}

bool TftpAckPacket::parse(const uint8_t* data, size_t size) {
    if (!TftpPacket::parse(data, size)) {
        return false;
    }
    
    if (size < 4) {
        return false;
    }
    
    return parseBlockNumber(data, 2);
}

std::vector<uint8_t> TftpAckPacket::serialize() const {
    std::vector<uint8_t> result = TftpPacket::serialize();
    serializeBlockNumber(result);
    return result;
}

bool TftpAckPacket::isValid() const {
    return TftpPacket::isValid() && opcode_ == TftpOpcode::ACK;
}

std::string TftpAckPacket::getTypeString() const {
    return "TFTP_ACK";
}

bool TftpAckPacket::parseBlockNumber(const uint8_t* data, size_t offset) {
    // Note: This is a basic implementation - in a real implementation,
    // we would need to pass the total data size as a parameter
    // For now, just assume we have enough data
    block_number_ = (data[offset] << 8) | data[offset + 1];
    return true;
}

void TftpAckPacket::serializeBlockNumber(std::vector<uint8_t>& data) const {
    data.push_back((block_number_ >> 8) & 0xFF);
    data.push_back(block_number_ & 0xFF);
}

// TftpErrorPacket implementation
TftpErrorPacket::TftpErrorPacket(TftpError error_code, const std::string& error_message)
    : TftpPacket(TftpOpcode::ERROR), error_code_(error_code), error_message_(error_message) {}

TftpErrorPacket::TftpErrorPacket(const uint8_t* data, size_t size)
    : TftpPacket(data, size) {
    parse(data, size);
}

TftpError TftpErrorPacket::getErrorCode() const {
    return error_code_;
}

void TftpErrorPacket::setErrorCode(TftpError error_code) {
    error_code_ = error_code;
}

std::string TftpErrorPacket::getErrorMessage() const {
    return error_message_;
}

void TftpErrorPacket::setErrorMessage(const std::string& error_message) {
    error_message_ = error_message;
}

bool TftpErrorPacket::parse(const uint8_t* data, size_t size) {
    if (!TftpPacket::parse(data, size)) {
        return false;
    }
    
    if (size < 4) {
        return false;
    }
    
    return parseErrorCode(data, 2);
}

std::vector<uint8_t> TftpErrorPacket::serialize() const {
    std::vector<uint8_t> result = TftpPacket::serialize();
    serializeErrorCode(result);
    result.insert(result.end(), error_message_.begin(), error_message_.end());
    result.push_back(0); // null terminator
    return result;
}

bool TftpErrorPacket::isValid() const {
    return TftpPacket::isValid() && opcode_ == TftpOpcode::ERROR;
}

std::string TftpErrorPacket::getTypeString() const {
    return "TFTP_ERROR";
}

bool TftpErrorPacket::parseErrorCode(const uint8_t* data, size_t offset) {
    // Note: This is a basic implementation - in a real implementation,
    // we would need to pass the total data size as a parameter
    // For now, just assume we have enough data
    uint16_t error_code_value = (data[offset] << 8) | data[offset + 1];
    error_code_ = static_cast<TftpError>(error_code_value);
    
    return true;
}

void TftpErrorPacket::serializeErrorCode(std::vector<uint8_t>& data) const {
    uint16_t error_code_value = static_cast<uint16_t>(error_code_);
    data.push_back((error_code_value >> 8) & 0xFF);
    data.push_back(error_code_value & 0xFF);
}

} // namespace simple_tftpd
