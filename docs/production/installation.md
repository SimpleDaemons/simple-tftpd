# Production Version Installation Guide

**Version:** 0.2.0-beta  
**License:** Apache 2.0

---

## Overview

This guide covers installing the Production Version of Simple TFTP Daemon. The Production Version is the open-source edition licensed under Apache 2.0.

## Prerequisites

### System Requirements

- **Operating System**: Linux (kernel 3.10+), macOS 10.15+, or Windows 10/11
- **Architecture**: x86_64, ARM64 (macOS), or compatible
- **Memory**: 256 MB RAM minimum, 1 GB recommended
- **Disk Space**: 50 MB for installation, additional space for files and logs
- **Network**: IPv4/IPv6 support

### Required Dependencies

- **C++ Runtime**: Standard C++17 runtime libraries
- **Network Libraries**: Standard socket libraries
- **File System**: Read/write access to designated directories

### Optional Dependencies

- **JSON Support**: jsoncpp (for JSON configuration)
- **YAML Support**: yaml-cpp (for YAML configuration)

## Installation Methods

### Method 1: Package Installation (Recommended)

#### Ubuntu/Debian
```bash
# Add repository (when available)
curl -fsSL https://repos.simpledaemons.com/ubuntu/gpg | sudo apt-key add -
echo "deb https://repos.simpledaemons.com/ubuntu stable main" | sudo tee /etc/apt/sources.list.d/simple-tftpd.list

# Install
sudo apt update
sudo apt install simple-tftpd
```

#### CentOS/RHEL
```bash
# Add repository (when available)
sudo yum install https://repos.simpledaemons.com/rhel/simple-tftpd-release.rpm
sudo yum install simple-tftpd
```

#### macOS (Homebrew)
```bash
# Install from Homebrew (when available)
brew install simple-tftpd
```

### Method 2: Build from Source

#### Step 1: Install Build Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config libjsoncpp-dev
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git jsoncpp-devel
```

**macOS:**
```bash
brew install cmake pkg-config jsoncpp
```

**Windows:**
- Install Visual Studio 2019 or later with C++ development tools
- Install CMake 3.16+ from [cmake.org](https://cmake.org/download/)

#### Step 2: Clone Repository
```bash
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
```

#### Step 3: Build
```bash
# Create build directory
mkdir build
cd build

# Configure
cmake -DBUILD_VERSION=production ..

# Build
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

#### Step 4: Install
```bash
# Install to system directories
sudo make install

# Or install to custom location
cmake -DCMAKE_INSTALL_PREFIX=/opt/simple-tftpd ..
make install
```

### Method 3: Docker

```bash
# Pull image
docker pull simpledaemons/simple-tftpd:production

# Run container
docker run -d \
  --name simple-tftpd \
  --net host \
  -v /var/tftp:/var/tftp \
  -v /etc/simple-tftpd:/etc/simple-tftpd \
  simpledaemons/simple-tftpd:production
```

## Post-Installation

### Verify Installation

```bash
# Check version
simple-tftpd --version

# Check help
simple-tftpd --help

# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

### Create Configuration

```bash
# Create configuration directory
sudo mkdir -p /etc/simple-tftpd

# Copy example configuration
sudo cp /usr/share/simple-tftpd/simple-tftpd.conf.example /etc/simple-tftpd/simple-tftpd.conf

# Or create from template
sudo tee /etc/simple-tftpd/simple-tftpd.conf > /dev/null <<EOF
{
  "network": {
    "listen_address": "0.0.0.0",
    "listen_port": 69,
    "ipv6_enabled": true
  },
  "filesystem": {
    "root_directory": "/var/tftp",
    "allowed_directories": []
  },
  "security": {
    "read_enabled": true,
    "write_enabled": false,
    "max_file_size": 104857600
  },
  "performance": {
    "block_size": 512,
    "timeout": 5,
    "window_size": 1
  },
  "logging": {
    "level": "INFO",
    "log_file": "/var/log/simple-tftpd.log",
    "console_logging": false
  }
}
EOF
```

### Create TFTP Root Directory

```bash
# Create root directory
sudo mkdir -p /var/tftp

# Set permissions
sudo chmod 755 /var/tftp

# Create tftp user (if it doesn't exist)
sudo useradd -r -s /bin/false -d /var/tftp tftp
sudo chown tftp:tftp /var/tftp
```

### Create System Service

#### systemd (Linux)

Create `/etc/systemd/system/simple-tftpd.service`:

```ini
[Unit]
Description=Simple TFTP Daemon (Production)
After=network.target

[Service]
Type=simple
User=tftp
ExecStart=/usr/bin/simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd
sudo systemctl status simple-tftpd
```

#### launchd (macOS)

Create `/Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.simpledaemons.simple-tftpd</string>
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

Load service:
```bash
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist
```

## Configuration

See the [Configuration Guide](configuration.md) for detailed configuration options.

## Verification

### Test Server

```bash
# Create test file
echo "Test file" | sudo tee /var/tftp/test.txt

# Test download
tftp localhost
tftp> get test.txt
tftp> quit

# Verify file
cat test.txt
```

### Check Logs

```bash
# System logs
sudo journalctl -u simple-tftpd -f

# Log file
tail -f /var/log/simple-tftpd.log
```

## Troubleshooting

### Common Issues

#### Port Already in Use
```bash
# Check what's using port 69
sudo netstat -tulpn | grep :69

# Stop conflicting service
sudo systemctl stop tftpd
```

#### Permission Denied
```bash
# Check permissions
ls -la /var/tftp

# Fix permissions
sudo chmod 755 /var/tftp
sudo chown tftp:tftp /var/tftp
```

#### Configuration Errors
```bash
# Validate configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check JSON syntax
cat /etc/simple-tftpd/simple-tftpd.conf | python -m json.tool
```

## Next Steps

- **[Configuration Guide](configuration.md)** - Configure the server
- **[Quick Start Guide](../shared/getting-started/quick-start.md)** - Get running quickly
- **[Deployment Guide](deployment.md)** - Production deployment
- **[Troubleshooting Guide](../shared/troubleshooting/README.md)** - Common issues

## Support

- **Documentation**: [Complete Documentation](../README.md)
- **GitHub Issues**: [Report Issues](https://github.com/SimpleDaemons/simple-tftpd/issues)
- **Troubleshooting**: [Troubleshooting Guide](../shared/troubleshooting/README.md)

---

**Last Updated**: December 2024
