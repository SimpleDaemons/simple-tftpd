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

#include "simple_tftpd/logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace simple_tftpd {

Logger::Logger(const std::string& log_file, LogLevel level, bool enable_console)
    : log_file_(log_file), level_(level), enable_console_(enable_console) {
    
    if (!log_file_.empty()) {
        log_stream_.open(log_file_, std::ios::app);
        if (!log_stream_.is_open()) {
            // Fall back to console only if file can't be opened
            enable_console_ = true;
            log_file_.clear();
        }
    }
}

Logger::~Logger() {
    if (log_stream_.is_open()) {
        log_stream_.close();
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

LogLevel Logger::getLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return level_;
}

void Logger::setConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    enable_console_ = enable;
}

bool Logger::setLogFile(const std::string& log_file) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (log_stream_.is_open()) {
        log_stream_.close();
    }
    
    log_file_ = log_file;
    
    if (!log_file_.empty()) {
        log_stream_.open(log_file_, std::ios::app);
        if (!log_stream_.is_open()) {
            return false;
        }
    }
    
    return true;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < level_) {
        return;
    }
    
    writeLog(level, message);
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string formatted_message = formatMessage(level, message);
    
    if (enable_console_) {
        std::cout << formatted_message << std::endl;
    }
    
    if (log_stream_.is_open()) {
        log_stream_ << formatted_message << std::endl;
        log_stream_.flush();
    }
}

std::string Logger::formatMessage(LogLevel level, const std::string& message) {
    std::stringstream ss;
    ss << "[" << getTimestamp() << "] "
       << "[" << levelToString(level) << "] "
       << message;
    
    return ss.str();
}

} // namespace simple_tftpd
