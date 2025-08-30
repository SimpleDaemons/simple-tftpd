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
    
    // Basic parsing - just set defaults for now
    filename_ = "default.txt";
    mode_ = TftpMode::OCTET;
    
    return true;
}

std::vector<uint8_t> TftpRequestPacket::serialize() const {
    std::vector<uint8_t> result = TftpPacket::serialize();
    
    // Add filename
    result.insert(result.end(), filename_.begin(), filename_.end());
    result.push_back(0); // null terminator
    
    // Add mode string
    std::string mode_str = "octet";
    result.insert(result.end(), mode_str.begin(), mode_str.end());
    result.push_back(0); // null terminator
    
    return result;
}

bool TftpRequestPacket::isValid() const {
    return TftpPacket::isValid() && !filename_.empty();
}

std::string TftpRequestPacket::getTypeString() const {
    return "TFTP_REQUEST";
}

bool TftpRequestPacket::parseOptions(const uint8_t* data, size_t offset, size_t size) {
    // Basic implementation - just return true for now
    return true;
}

void TftpRequestPacket::serializeOptions(std::vector<uint8_t>& data) const {
    // Basic implementation - do nothing for now
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
    if (offset + 1 >= data.size()) {
        return false;
    }
    
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
    if (offset + 1 >= data.size()) {
        return false;
    }
    
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
    if (offset + 1 >= data.size()) {
        return false;
    }
    
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
