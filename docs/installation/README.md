# Installation Guide

This guide covers installing the Simple TFTP Daemon on various platforms.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation Methods](#installation-methods)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Post-Installation](#post-installation)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### System Requirements

- **Operating System**: Linux, macOS 12.0+, or Windows 10+
- **Architecture**: x86_64, ARM64 (macOS), or compatible
- **Memory**: 64MB RAM minimum, 256MB recommended
- **Disk Space**: 10MB for installation, additional space for logs and files

### Development Dependencies

- **Compiler**: C++17 compatible compiler
- **CMake**: Version 3.16 or higher
- **Build Tools**: Make, Ninja, or equivalent
- **Git**: For source code management

### Runtime Dependencies

- **System Libraries**: Standard C++ runtime libraries
- **Network**: IPv4/IPv6 networking support
- **File System**: Read/write access to designated directories

## Installation Methods

### Method 1: Build from Source (Recommended)

#### Step 1: Clone the Repository

```bash
git clone https://github.com/your-username/simple-tftpd.git
cd simple-tftpd
```

#### Step 2: Build the Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

#### Step 3: Install

```bash
# Install to system directories
sudo make install

# Or install to custom location
cmake -DCMAKE_INSTALL_PREFIX=/opt/simple-tftpd ..
make install
```

### Method 2: Using Build Scripts

#### macOS

```bash
# Use the provided build script
./scripts/build-macos.sh --all

# Or with specific options
./scripts/build-macos.sh --clean --test --install
```

#### Linux

```bash
# Use the provided build script
./scripts/build-linux.sh --all
```

### Method 3: Package Managers

#### Ubuntu/Debian

```bash
# Add repository (when available)
sudo apt update
sudo apt install simple-tftpd
```

#### CentOS/RHEL/Fedora

```bash
# Install from repository (when available)
sudo yum install simple-tftpd
# or
sudo dnf install simple-tftpd
```

#### macOS (Homebrew)

```bash
# Install from Homebrew (when available)
brew install simple-tftpd
```

## Platform-Specific Instructions

### Linux

#### Ubuntu 20.04+

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git pkg-config

# Build and install
git clone https://github.com/your-username/simple-tftpd.git
cd simple-tftpd
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

#### CentOS 8/RHEL 8+

```bash
# Install dependencies
sudo yum groupinstall "Development Tools"
sudo yum install cmake3 git

# Build and install
git clone https://github.com/your-username/simple-tftpd.git
cd simple-tftpd
mkdir build && cd build
cmake3 ..
make -j$(nproc)
sudo make install
```

### macOS

#### Prerequisites

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake pkg-config git
```

#### Build and Install

```bash
# Clone and build
git clone https://github.com/your-username/simple-tftpd.git
cd simple-tftpd

# Use the provided build script
./scripts/build-macos.sh --all
```

### Windows

#### Prerequisites

1. **Visual Studio**: Install Visual Studio 2019 or later with C++ development tools
2. **CMake**: Install CMake 3.16+ from [cmake.org](https://cmake.org/download/)
3. **Git**: Install Git for Windows

#### Build and Install

```cmd
# Clone repository
git clone https://github.com/your-username/simple-tftpd.git
cd simple-tftpd

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release

# Install (optional)
cmake --install . --prefix C:\Program Files\simple-tftpd
```

## Post-Installation

### Verify Installation

```bash
# Check if executable is available
simple-tftpd --version

# Check help
simple-tftpd --help
```

### Create Configuration Directory

```bash
# Create configuration directory
sudo mkdir -p /etc/simple-tftpd

# Copy example configuration
sudo cp config/simple-tftpd.conf.example /etc/simple-tftpd/simple-tftpd.conf

# Create TFTP root directory
sudo mkdir -p /var/tftp
sudo chown tftp:tftp /var/tftp  # If tftp user exists
```

### Create System Service

#### systemd (Linux)

Create `/etc/systemd/system/simple-tftpd.service`:

```ini
[Unit]
Description=Simple TFTP Daemon
After=network.target

[Service]
Type=simple
User=tftp
ExecStart=/usr/bin/simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd
sudo systemctl status simple-tftpd
```

#### launchd (macOS)

Create `/Library/LaunchDaemons/com.simple-tftpd.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.simple-tftpd</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/simple-tftpd</string>
        <string>start</string>
        <string>--config</string>
        <string>/etc/simple-tftpd/simple-tftpd.conf</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

Load the service:

```bash
sudo launchctl load /Library/LaunchDaemons/com.simple-tftpd.plist
```

## Troubleshooting

### Common Issues

#### Build Failures

**Problem**: CMake configuration fails
```bash
-- Could not find jsoncpp
```

**Solution**: Install jsoncpp development package
```bash
# Ubuntu/Debian
sudo apt install libjsoncpp-dev

# CentOS/RHEL
sudo yum install jsoncpp-devel

# macOS
brew install jsoncpp
```

**Problem**: Compilation errors
```bash
error: 'std::chrono' has not been declared
```

**Solution**: Ensure C++17 support is enabled
```bash
cmake -DCMAKE_CXX_STANDARD=17 ..
```

#### Runtime Issues

**Problem**: Permission denied errors
```bash
simple-tftpd: error: cannot open log file
```

**Solution**: Check file permissions and ownership
```bash
sudo chown tftp:tftp /var/log/simple-tftpd.log
sudo chmod 644 /var/log/simple-tftpd.log
```

**Problem**: Port already in use
```bash
simple-tftpd: error: bind failed: Address already in use
```

**Solution**: Check for existing TFTP services
```bash
sudo netstat -tulpn | grep :69
sudo systemctl stop tftpd  # Stop system TFTP service
```

### Getting Help

- **Check Logs**: Look for error messages in system logs
- **Verbose Mode**: Run with `--verbose` for detailed output
- **Configuration**: Verify configuration file syntax
- **Dependencies**: Ensure all required libraries are installed
- **Issues**: Report problems on GitHub with detailed information

## Next Steps

After successful installation:

1. **[Configuration Guide](../configuration/README.md)** - Configure the daemon
2. **[User Guide](../user-guide/README.md)** - Learn how to use the service
3. **[Examples](../examples/README.md)** - See practical usage examples

## Support

For installation support:

- Check this documentation first
- Search existing GitHub issues
- Create a new issue with:
  - Operating system and version
  - Installation method used
  - Complete error messages
  - System information (architecture, compiler version)
