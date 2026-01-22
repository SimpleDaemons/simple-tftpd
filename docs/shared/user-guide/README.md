# User Guide

This guide covers how to use the Simple TFTP Daemon for file transfers.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Command Line Interface](#command-line-interface)
- [Configuration](#configuration)
- [Basic Operations](#basic-operations)
- [Advanced Features](#advanced-features)
- [Monitoring and Management](#monitoring-and-management)
- [Troubleshooting](#troubleshooting)

## Overview

The Simple TFTP Daemon (simple-tftpd) is a lightweight, cross-platform TFTP server that implements the Trivial File Transfer Protocol (RFC 1350). It's designed to be simple to use while providing robust file transfer capabilities.

### What is TFTP?

TFTP (Trivial File Transfer Protocol) is a simple protocol for transferring files between systems. It's commonly used for:

- Network booting (PXE)
- Firmware updates
- Configuration file distribution
- Simple file sharing

### Key Features

- **Cross-platform**: Runs on Linux, macOS, and Windows
- **Configurable**: JSON-based configuration with command-line overrides
- **Secure**: Access controls and file restrictions
- **Efficient**: Configurable block sizes and timeouts
- **Logging**: Comprehensive logging with configurable levels

## Quick Start

### 1. Start the Server

```bash
# Start with default settings
simple-tftpd start

# Start with custom configuration
simple-tftpd start --config /etc/simple-tftpd/config.json

# Start with command-line options
simple-tftpd start --listen 0.0.0.0 --port 69 --root /var/tftp
```

### 2. Test the Connection

```bash
# From another machine, test file download
tftp localhost 69
tftp> get testfile.txt
tftp> quit
```

### 3. Check Status

```bash
# Check server status
simple-tftpd status

# View server statistics
simple-tftpd stats
```

## Command Line Interface

### Basic Syntax

```bash
simple-tftpd [OPTIONS] [COMMAND] [ARGS...]
```

### Global Options

| Option | Short | Description |
|--------|-------|-------------|
| `--help` | `-h` | Show help message |
| `--version` | `-v` | Show version information |
| `--config` | `-c` | Use specified configuration file |
| `--verbose` | `-V` | Enable verbose logging |
| `--daemon` | `-d` | Run as daemon |
| `--foreground` | `-f` | Run in foreground |

### Commands

#### `start` - Start the TFTP Server

```bash
simple-tftpd start [OPTIONS]
```

**Options:**
- `--listen ADDR` - Listen on specific address
- `--port PORT` - Listen on specific port
- `--root DIR` - Set root directory for file operations

**Examples:**
```bash
# Start with default settings
simple-tftpd start

# Start on specific interface and port
simple-tftpd start --listen 192.168.1.100 --port 6969

# Start with custom root directory
simple-tftpd start --root /home/user/tftp-files
```

#### `stop` - Stop the TFTP Server

```bash
simple-tftpd stop
```

**Note**: This command requires the server to be running as a service.

#### `status` - Show Server Status

```bash
simple-tftpd status
```

**Output Example:**
```
TFTP Server Status:
  Running: Yes
  Listen Address: 0.0.0.0
  Listen Port: 69
  IPv6 Enabled: Yes
  Active Connections: 2
  Uptime: 3600 seconds
```

#### `stats` - Show Server Statistics

```bash
simple-tftpd stats
```

**Output Example:**
```
TFTP Server Statistics:
  Total Connections: 150
  Active Connections: 2
  Total Read Requests: 120
  Total Write Requests: 30
  Total Bytes Transferred: 1,048,576
  Total Errors: 5
  Uptime: 1 hour, 0 minutes
```

#### `connections` - List Active Connections

```bash
simple-tftpd connections
```

**Output Example:**
```
Connection: 192.168.1.100:12345
  State: TRANSFERRING
  Filename: firmware.bin
  Bytes Transferred: 51200
  Duration: 45 seconds

Connection: 192.168.1.101:54321
  State: CONNECTED
  Filename: config.txt
  Bytes Transferred: 0
  Duration: 10 seconds
```

#### `reload` - Reload Configuration

```bash
simple-tftpd reload
```

Reloads the configuration file without stopping the server.

#### `test` - Test Configuration

```bash
simple-tftpd test
```

Validates the configuration file and reports any issues.

## Configuration

### Configuration File Format

The daemon uses JSON configuration files. Example configuration:

```json
{
    "network": {
        "listen_address": "0.0.0.0",
        "listen_port": 69,
        "ipv6_enabled": true
    },
    "filesystem": {
        "root_directory": "/var/tftp",
        "allowed_directories": [
            "/var/tftp/public",
            "/var/tftp/private"
        ]
    },
    "security": {
        "read_enabled": true,
        "write_enabled": false,
        "max_file_size": 104857600,
        "overwrite_protection": true
    },
    "performance": {
        "block_size": 512,
        "timeout": 5,
        "window_size": 1
    },
    "logging": {
        "level": "INFO",
        "log_file": "/var/log/simple-tftpd.log",
        "console_logging": true
    }
}
```

### Configuration Options

#### Network Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `listen_address` | string | "0.0.0.0" | IP address to listen on |
| `listen_port` | integer | 69 | Port to listen on |
| `ipv6_enabled` | boolean | true | Enable IPv6 support |

#### File System Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `root_directory` | string | "/var/tftp" | Root directory for file operations |
| `allowed_directories` | array | [] | List of allowed directory paths |

#### Security Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `read_enabled` | boolean | true | Allow file downloads |
| `write_enabled` | boolean | false | Allow file uploads |
| `max_file_size` | integer | 104857600 | Maximum file size (100MB) |
| `overwrite_protection` | boolean | true | Prevent file overwrites |

#### Performance Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `block_size` | integer | 512 | TFTP block size in bytes |
| `timeout` | integer | 5 | Timeout in seconds |
| `window_size` | integer | 1 | Window size for transfers |

#### Logging Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `level` | string | "INFO" | Log level (DEBUG, INFO, WARNING, ERROR, FATAL) |
| `log_file` | string | "" | Path to log file |
| `console_logging` | boolean | true | Enable console output |

### Environment Variables

You can override configuration using environment variables:

```bash
export SIMPLE_TFTPD_LISTEN_ADDRESS=192.168.1.100
export SIMPLE_TFTPD_LISTEN_PORT=6969
export SIMPLE_TFTPD_ROOT_DIRECTORY=/home/user/tftp
export SIMPLE_TFTPD_LOG_LEVEL=DEBUG

simple-tftpd start
```

## Basic Operations

### Starting the Server

#### Interactive Mode

```bash
# Start in foreground with verbose logging
simple-tftpd --verbose start

# Start with custom configuration
simple-tftpd start --config /etc/simple-tftpd/config.json
```

#### Daemon Mode

```bash
# Start as daemon
simple-tftpd --daemon start

# Start as daemon with custom config
simple-tftpd --daemon start --config /etc/simple-tftpd/config.json
```

### Stopping the Server

```bash
# Stop the server
simple-tftpd stop

# Or use system service commands
sudo systemctl stop simple-tftpd
```

### Checking Server Status

```bash
# Check if server is running
simple-tftpd status

# Check system service status
sudo systemctl status simple-tftpd
```

## Advanced Features

### File Access Control

Configure allowed directories to restrict file access:

```json
{
    "filesystem": {
        "allowed_directories": [
            "/var/tftp/public",
            "/var/tftp/firmware",
            "/var/tftp/configs"
        ]
    }
}
```

### Transfer Modes

The daemon supports standard TFTP transfer modes:

- **netascii**: Text file transfer with line ending conversion
- **octet**: Binary file transfer (default)
- **mail**: Mail transfer (deprecated)

### Block Size Optimization

Optimize transfer performance by adjusting block size:

```json
{
    "performance": {
        "block_size": 1024,
        "timeout": 10
    }
}
```

**Note**: Larger block sizes improve performance but may cause issues with some TFTP clients.

### IPv6 Support

Enable IPv6 support for modern networks:

```json
{
    "network": {
        "ipv6_enabled": true,
        "listen_address": "::"
    }
}
```

## Monitoring and Management

### Real-time Monitoring

```bash
# Watch server status
watch -n 1 'simple-tftpd status'

# Monitor log files
tail -f /var/log/simple-tftpd.log
```

### Performance Metrics

```bash
# Get transfer statistics
simple-tftpd stats

# Monitor active connections
simple-tftpd connections
```

### Log Analysis

```bash
# Search for errors
grep ERROR /var/log/simple-tftpd.log

# Count successful transfers
grep "Transfer completed" /var/log/simple-tftpd.log | wc -l

# Monitor specific client activity
grep "192.168.1.100" /var/log/simple-tftpd.log
```

## Troubleshooting

### Common Issues

#### Server Won't Start

**Problem**: Port already in use
```bash
simple-tftpd: error: bind failed: Address already in use
```

**Solution**: Check for existing services
```bash
sudo netstat -tulpn | grep :69
sudo systemctl stop tftpd  # Stop system TFTP service
```

**Problem**: Permission denied
```bash
simple-tftpd: error: cannot open log file
```

**Solution**: Check file permissions
```bash
sudo chown tftp:tftp /var/log/simple-tftpd.log
sudo chmod 644 /var/log/simple-tftpd.log
```

#### File Transfer Issues

**Problem**: File not found
```bash
tftp> get nonexistent.txt
Error code 1: File not found
```

**Solution**: Check file exists and permissions
```bash
ls -la /var/tftp/
sudo chmod 644 /var/tftp/filename.txt
```

**Problem**: Access violation
```bash
tftp> put protected.txt
Error code 2: Access violation
```

**Solution**: Check write permissions and allowed directories
```bash
sudo chmod 755 /var/tftp/
# Verify configuration allows writes
```

### Debug Mode

Enable verbose logging for troubleshooting:

```bash
# Start with debug logging
simple-tftpd --verbose start

# Or set log level in configuration
{
    "logging": {
        "level": "DEBUG"
    }
}
```

### Getting Help

- **Check Logs**: Look for error messages in log files
- **Verify Configuration**: Use `simple-tftpd test` to validate config
- **Check Permissions**: Ensure proper file and directory permissions
- **Network Issues**: Verify firewall and network configuration
- **Report Issues**: Create GitHub issue with detailed information

## Next Steps

After mastering basic usage:

1. **[Examples](../examples/README.md)** - See practical usage scenarios
2. **[Configuration Reference](../configuration/README.md)** - Detailed configuration options
3. **[Development Guide](../development/README.md)** - Contribute to the project

## Support

For user support:

- Check this documentation first
- Search existing GitHub issues
- Create a new issue with:
  - Operating system and version
  - simple-tftpd version
  - Complete error messages
  - Configuration file contents
  - Steps to reproduce the issue
