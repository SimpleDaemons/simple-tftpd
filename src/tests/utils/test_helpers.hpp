#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace simple_tftpd {
namespace test {

class TestHelpers {
public:
    TestHelpers();
    ~TestHelpers();
    
    std::string createTempDirectory();
    std::string createTestFile(const std::string& filename, const std::string& content);
    std::string createTestFile(const std::string& filename, size_t size);
    bool fileExists(const std::string& filepath);
    std::string readFile(const std::string& filepath);
    size_t getFileSize(const std::string& filepath);
    void cleanup();
    std::string getTestDirectory() const;
    std::string generateRandomString(size_t length);
    std::vector<uint8_t> generateRandomData(size_t size);
    bool compareFiles(const std::string& file1, const std::string& file2);
    std::string getNetworkInterface();
    bool isPortAvailable(uint16_t port);
    uint16_t findAvailablePort(uint16_t start_port);

private:
    std::string test_dir_;
};

} // namespace test
} // namespace simple_tftpd
