# Getting Started with Simple TFTP Daemon

Welcome to Simple TFTP Daemon! This guide will help you get started quickly.

## Quick Navigation

- **[Installation Guide](installation/README.md)** - Install Simple TFTP Daemon on your platform
- **[Quick Start Guide](quick-start.md)** - Get running in minutes
- **[First Steps](first-steps.md)** - Basic configuration and usage

## What is Simple TFTP Daemon?

Simple TFTP Daemon is a lightweight, cross-platform TFTP (Trivial File Transfer Protocol) server written in C++17. It provides:

- Complete TFTP protocol implementation (RFC 1350, RFC 2347)
- Windowed transfers with sliding window
- TFTP options support (blksize, timeout, tsize, windowsize)
- Security features (access control, file filtering, IP filtering)
- Multi-format configuration (JSON, YAML, INI)
- Hot reload configuration
- Cross-platform support (Linux, macOS, Windows)

## Installation

Choose your platform:

### Linux
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install simple-tftpd

# CentOS/RHEL
sudo yum install simple-tftpd

# Or build from source
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### macOS
```bash
# Homebrew (when available)
brew install simple-tftpd

# Or build from source
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
./scripts/build-macos.sh --all
```

### Windows
```cmd
# Build from source
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

For detailed installation instructions, see the [Installation Guide](installation/README.md).

## Quick Start

1. **Install** Simple TFTP Daemon (see above)
2. **Create configuration**:
   ```bash
   sudo mkdir -p /etc/simple-tftpd
   sudo cp config/simple-tftpd.conf.example /etc/simple-tftpd/simple-tftpd.conf
   ```
3. **Create TFTP root directory**:
   ```bash
   sudo mkdir -p /var/tftp
   ```
4. **Start the server**:
   ```bash
   sudo simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
   ```

For a complete quick start guide, see [Quick Start Guide](quick-start.md).

## Next Steps

- **[First Steps](first-steps.md)** - Learn basic configuration and usage
- **[Configuration Reference](../configuration/README.md)** - Complete configuration guide
- **[Production Deployment](../../production/deployment.md)** - Deploy to production
- **[Troubleshooting](../troubleshooting/README.md)** - Common issues and solutions

## Documentation

- **[Complete Documentation Index](../../README.md)** - All documentation
- **[Production Version](../../production/README.md)** - Production version docs
- **[Enterprise Version](../../enterprise/README.md)** - Enterprise version docs
- **[Datacenter Version](../../datacenter/README.md)** - Datacenter version docs

## Support

- **GitHub Issues**: [Report Issues](https://github.com/SimpleDaemons/simple-tftpd/issues)
- **Documentation**: [Full Documentation](../../README.md)
- **Troubleshooting**: [Troubleshooting Guide](../troubleshooting/README.md)

---

**Last Updated**: December 2024
