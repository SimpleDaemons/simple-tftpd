# Development Guide

This guide covers how to set up a development environment and contribute to the Simple TFTP Daemon project.

## Table of Contents

- [Development Environment](#development-environment)
- [Project Structure](#project-structure)
- [Building from Source](#building-from-source)
- [Code Style and Standards](#code-style-and-standards)
- [Testing](#testing)
- [Debugging](#debugging)
- [Contributing](#contributing)
- [Release Process](#release-process)

## Development Environment

### Prerequisites

- **Operating System**: Linux, macOS 12.0+, or Windows 10+
- **Compiler**: C++17 compatible compiler
- **CMake**: Version 3.16 or higher
- **Git**: Version 2.20 or higher
- **Build Tools**: Make, Ninja, or equivalent

### Required Software

#### Linux

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git pkg-config

# CentOS/RHEL/Fedora
sudo yum groupinstall "Development Tools"
sudo yum install cmake3 git
# or
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git
```

#### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake pkg-config git
```

#### Windows

1. **Visual Studio**: Install Visual Studio 2019 or later with C++ development tools
2. **CMake**: Install CMake 3.16+ from [cmake.org](https://cmake.org/download/)
3. **Git**: Install Git for Windows

### Optional Dependencies

- **jsoncpp**: For JSON configuration parsing
- **Google Test**: For unit testing
- **clang-format**: For code formatting
- **valgrind**: For memory leak detection (Linux)

## Project Structure

```
simple-tftpd/
├── cmake/                 # CMake configuration files
├── config/                # Configuration files and examples
├── docs/                  # Documentation
├── include/               # Header files
│   └── simple_tftpd/     # Public API headers
├── scripts/               # Build and utility scripts
├── src/                   # Source files
│   ├── core/             # Core TFTP functionality
│   ├── utils/            # Utility classes
│   ├── tests/            # Test suite
│   └── examples/         # Example programs
├── tools/                 # Development tools
├── CMakeLists.txt         # Main CMake file
├── Makefile               # Build convenience targets
└── README.md             # Project overview
```

### Key Directories

#### `include/simple_tftpd/`

Contains all public header files:

- `platform.hpp` - Platform abstraction layer
- `logger.hpp` - Logging system
- `tftp_config.hpp` - Configuration management
- `tftp_packet.hpp` - TFTP packet handling
- `tftp_connection.hpp` - Connection management
- `tftp_server.hpp` - Main server class

#### `src/core/`

Core TFTP implementation:

- `tftp_server.cpp` - Main server implementation
- `tftp_connection.cpp` - Connection handling
- `tftp_packet.cpp` - Packet parsing and creation

#### `src/utils/`

Utility classes:

- `logger.cpp` - Logging implementation
- `config_parser.cpp` - Configuration parsing

#### `src/tests/`

Test suite:

- `test_main.cpp` - Test entry point
- `test_helpers.cpp` - Test utilities

## Quick Start

1. **[Setup Guide](SETUP.md)** - Set up your development environment
2. **[Build Guide](BUILD_GUIDE.md)** - Build commands and reference
3. Continue reading this guide for development workflow

## Building from Source

See [Build Guide](BUILD_GUIDE.md) for complete build instructions.

### Quick Build

```bash
# Clone repository
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd

# Create build directory
mkdir build && cd build

# Configure and build Production version
cmake -DBUILD_VERSION=production ..
make -j$(nproc)  # Linux/macOS
```

### Version-Specific Builds

```bash
# Production version
cmake -DBUILD_VERSION=production ..

# Enterprise version
cmake -DBUILD_VERSION=enterprise ..

# Datacenter version
cmake -DBUILD_VERSION=datacenter ..
```

### Using Build Scripts

#### macOS

```bash
# Full build process
./scripts/build-macos.sh --all

# Clean build
./scripts/build-macos.sh --clean --test

# Debug build
./scripts/build-macos.sh --debug
```

#### Linux

```bash
# Full build process
./scripts/build-linux.sh --all

# Custom options
./scripts/build-linux.sh --clean --install
```

### IDE Integration

#### Visual Studio Code

1. Install C/C++ extension
2. Open project folder
3. Configure `c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/src"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

#### CLion

1. Open project folder
2. CLion will automatically detect CMake project
3. Configure build options in CMake settings

#### Visual Studio

1. Open project folder
2. Use CMake integration
3. Configure build settings in CMakeSettings.json

## Code Style and Standards

### C++ Standards

- **Language**: C++17
- **Compiler**: GCC 7+, Clang 5+, MSVC 2017+
- **Platform**: Cross-platform (Linux, macOS, Windows)

### Naming Conventions

#### Classes and Types

```cpp
// PascalCase for class names
class TftpServer;
class TftpConnection;

// PascalCase for enums
enum class TftpOpcode;
enum class LogLevel;
```

#### Functions and Methods

```cpp
// camelCase for methods
void startServer();
bool isValidPacket();
std::string getStatus();
```

#### Variables and Members

```cpp
// camelCase for variables
std::string serverName;
int connectionCount;

// Underscore suffix for member variables
class TftpServer {
private:
    std::string server_name_;
    int connection_count_;
};
```

#### Constants

```cpp
// UPPER_SNAKE_CASE for constants
constexpr size_t MAX_PACKET_SIZE = 512;
constexpr port_t DEFAULT_PORT = 69;
```

### Code Formatting

#### Using clang-format

```bash
# Format all source files
make format

# Check formatting without changes
make check-style
```

#### Formatting Rules

```yaml
# .clang-format
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 120
AccessModifierOffset: -4
NamespaceIndentation: None
```

### Documentation Standards

#### Header Documentation

```cpp
/**
 * @brief TFTP server main class
 *
 * This class manages the main TFTP server functionality including:
 * - Listening for incoming connections
 * - Managing multiple TFTP connections
 * - User authentication and authorization
 * - Virtual host support
 * - SSL/TLS support
 */
class TftpServer {
public:
    /**
     * @brief Start the TFTP server
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the TFTP server
     */
    void stop();
};
```

#### Implementation Documentation

```cpp
bool TftpServer::start() {
    // Check if server is already running
    if (running_.load()) {
        logger_->warning("Server is already running");
        return false;
    }

    // Initialize network components
    if (!initializeNetwork()) {
        logger_->error("Failed to initialize network");
        return false;
    }

    // Start listener thread
    startListenerThread();

    running_.store(true);
    logger_->info("TFTP server started successfully");
    return true;
}
```

## Testing

### Running Tests

```bash
# Build and run tests
make test

# Run tests from build directory
cd build
make test

# Run specific test
./bin/simple-tftpd-tests --gtest_filter=TftpPacketTest*
```

### Writing Tests

#### Test Structure

```cpp
#include <gtest/gtest.h>
#include "simple_tftpd/tftp_packet.hpp"

class TftpPacketTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test data
    }

    void TearDown() override {
        // Cleanup test data
    }
};

TEST_F(TftpPacketTest, CreateValidPacket) {
    TftpPacket packet(TftpOpcode::RRQ);

    EXPECT_EQ(packet.getOpcode(), TftpOpcode::RRQ);
    EXPECT_TRUE(packet.isValid());
}

TEST_F(TftpPacketTest, ParseInvalidData) {
    uint8_t invalid_data[] = {0x00}; // Too short

    TftpPacket packet(invalid_data, 1);

    EXPECT_FALSE(packet.isValid());
}
```

#### Test Categories

- **Unit Tests**: Test individual classes and methods
- **Integration Tests**: Test component interactions
- **Performance Tests**: Test performance characteristics
- **Memory Tests**: Test memory management

### Test Coverage

```bash
# Generate coverage report
cmake -DENABLE_COVERAGE=ON ..
make
make coverage

# View coverage report
open coverage/index.html
```

## Debugging

### Debug Build

```bash
# Create debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with debugger
gdb ./bin/simple-tftpd
```

### Logging

```cpp
// Enable debug logging
logger_->debug("Processing packet: " + packet.getTypeString());

// Log with context
logger_->info("Connection established from " + client_addr + ":" + std::to_string(client_port));

// Error logging with details
logger_->error("Failed to send packet: " + std::string(strerror(errno)));
```

### Debugging Tools

#### Valgrind (Linux/macOS)

```bash
# Memory leak detection
valgrind --leak-check=full --show-leak-kinds=all ./bin/simple-tftpd

# Memory error detection
valgrind --tool=memcheck ./bin/simple-tftpd
```

#### AddressSanitizer

```bash
# Build with AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=address ..
make
```

#### GDB/LLDB

```bash
# Set breakpoints
(gdb) break TftpServer::start
(gdb) break tftp_connection.cpp:45

# Run with arguments
(gdb) run start --config test.conf

# Inspect variables
(gdb) print server_name_
(gdb) print *config_
```

## Contributing

### Development Workflow

1. **Fork** the repository
2. **Create** a feature branch
3. **Make** your changes
4. **Test** your changes
5. **Commit** with clear messages
6. **Push** to your fork
7. **Create** a pull request

### Commit Message Format

```
type(scope): brief description

Detailed description of changes

- Bullet point of change 1
- Bullet point of change 2

Fixes #123
```

#### Commit Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Test additions/changes
- `chore`: Build/tool changes

#### Examples

```
feat(server): add IPv6 support

- Implement IPv6 socket binding
- Add IPv6 configuration options
- Update network initialization logic

Closes #45
```

```
fix(packet): resolve memory leak in packet parsing

- Fix memory allocation in TftpPacket constructor
- Add proper cleanup in destructor
- Update unit tests to verify fix

Fixes #67
```

### Code Review Process

1. **Self-review**: Test your changes thoroughly
2. **Pull request**: Create detailed PR description
3. **Review**: Address feedback from maintainers
4. **Merge**: Changes merged after approval

### Testing Requirements

- All new code must include tests
- Existing tests must pass
- Code coverage should not decrease
- Performance tests for new features

## Release Process

### Version Management

- **Semantic Versioning**: MAJOR.MINOR.PATCH
- **Development**: 0.x.x versions
- **Stable**: 1.0.0 and above

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] Changelog updated
- [ ] Version numbers updated
- [ ] Release notes prepared
- [ ] Binaries built and tested
- [ ] GitHub release created

### Building Releases

```bash
# Create release build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Create packages
make package

# Install to staging area
make install DESTDIR=staging
```

## Getting Help

### Development Support

- **Issues**: Report bugs or request features
- **Discussions**: Ask questions and share ideas
- **Wiki**: Development tips and tricks
- **Code Review**: Get feedback on your changes

### Resources

- **[API Reference](../api/README.md)** - Complete API documentation
- **[Architecture Overview](../architecture/README.md)** - System design details
- **[Testing Guide](../testing/README.md)** - Testing best practices
- **[Contributing Guidelines](../../CONTRIBUTING.md)** - Contribution guidelines

## Next Steps

After setting up your development environment:

1. **[API Reference](../api/README.md)** - Learn the codebase
2. **[Architecture Overview](../architecture/README.md)** - Understand the design
3. **[Testing Guide](../testing/README.md)** - Write effective tests
4. **[Examples](../examples/README.md)** - See practical implementations

## Support

For development support:

- Check this documentation first
- Search existing GitHub issues
- Create a new issue with:
  - Development environment details
  - Complete error messages
  - Steps to reproduce
  - Proposed solution or question
