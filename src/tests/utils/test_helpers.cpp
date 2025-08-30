#include "test_helpers.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

namespace simple_tftpd {
namespace test {

TestHelpers::TestHelpers() {
    test_dir_ = createTempDirectory();
}

TestHelpers::~TestHelpers() {
    cleanup();
}

std::string TestHelpers::createTempDirectory() {
    std::string temp_dir = "/tmp/simple-tftpd-test-";
    temp_dir += std::to_string(getpid());
    temp_dir += "-";
    temp_dir += std::to_string(rand());
    
    std::string cmd = "mkdir -p " + temp_dir;
    system(cmd.c_str());
    
    return temp_dir;
}

std::string TestHelpers::createTestFile(const std::string& filename, const std::string& content) {
    std::string filepath = test_dir_ + "/" + filename;
    
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << content;
        file << content;
        file.close();
        return filepath;
    }
    
    return "";
}

std::string TestHelpers::createTestFile(const std::string& filename, size_t size) {
    std::string filepath = test_dir_ + "/" + filename;
    
    std::ofstream file(filepath, std::ios::binary);
    if (file.is_open()) {
        for (size_t i = 0; i < size; ++i) {
            file.put(static_cast<char>(rand() % 256));
        }
        file.close();
        return filepath;
    }
    
    return "";
}

bool TestHelpers::fileExists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}

std::string TestHelpers::readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    return content;
}

size_t TestHelpers::getFileSize(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    return 0;
}

void TestHelpers::cleanup() {
    if (!test_dir_.empty()) {
        std::string cmd = "rm -rf " + test_dir_;
        system(cmd.c_str());
        test_dir_.clear();
    }
}

std::string TestHelpers::getTestDirectory() const {
    return test_dir_;
}

std::string TestHelpers::generateRandomString(size_t length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < size; ++i) {
        result += charset[rand() % charset.length()];
    }
    
    return result;
}

std::vector<uint8_t> TestHelpers::generateRandomData(size_t size) {
    std::vector<uint8_t> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; ++i) {
        data.push_back(static_cast<uint8_t>(rand() % 256));
    }
    
    return data;
}

bool TestHelpers::compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    
    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }
    
    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    
    if (f1.tellg() != f2.tellg()) {
        return false;
    }
    
    f1.seekg(0);
    f2.seekg(0);
    
    std::istreambuf_iterator<char> begin1(f1), end1;
    std::istreambuf_iterator<char> begin2(f2), end2;
    
    return std::equal(begin1, end1, begin2);
}

std::string TestHelpers::getNetworkInterface() {
    std::string interfaces[] = {"lo0", "en0", "eth0", "wlan0"};
    
    for (const auto& iface : interfaces) {
        std::string cmd = "ifconfig " + iface + " >/dev/null 2>&1";
        if (system(cmd.c_str()) == 0) {
            return iface;
        }
    }
    
    return "lo0";
}

bool TestHelpers::isPortAvailable(uint16_t port) {
    std::string cmd = "netstat -an | grep ':" + std::to_string(port) + "' | grep LISTEN >/dev/null 2>&1";
    return system(cmd.c_str()) != 0;
}

uint16_t TestHelpers::findAvailablePort(uint16_t start_port) {
    for (uint16_t port = start_port; port < start_port + 100; ++port) {
        if (isPortAvailable(port)) {
            return port;
        }
    }
    return 0;
}

} // namespace test
} // namespace simple_tftpd
