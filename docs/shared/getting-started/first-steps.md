# First Steps with Simple TFTP Daemon

Learn the basics of configuring and using Simple TFTP Daemon.

## Table of Contents

- [Basic Configuration](#basic-configuration)
- [File Operations](#file-operations)
- [Security Basics](#security-basics)
- [Common Use Cases](#common-use-cases)
- [Next Steps](#next-steps)

## Basic Configuration

### Understanding the Configuration File

The configuration file is in JSON format and organized into sections:

```json
{
  "network": { ... },      // Network settings
  "filesystem": { ... },   // File system settings
  "security": { ... },     // Security settings
  "performance": { ... },  // Performance tuning
  "logging": { ... }      // Logging configuration
}
```

### Essential Settings

#### Network Configuration
```json
{
  "network": {
    "listen_address": "0.0.0.0",  // Listen on all interfaces
    "listen_port": 69,             // Standard TFTP port
    "ipv6_enabled": true            // Enable IPv6 support
  }
}
```

#### File System Configuration
```json
{
  "filesystem": {
    "root_directory": "/var/tftp",  // TFTP root directory
    "allowed_directories": [],      // Empty = allow all subdirectories
    "create_directories": false     // Don't auto-create directories
  }
}
```

#### Security Configuration
```json
{
  "security": {
    "read_enabled": true,           // Allow file downloads
    "write_enabled": false,         // Disable file uploads
    "max_file_size": 104857600,     // 100 MB limit
    "overwrite_protection": true    // Prevent overwriting files
  }
}
```

## File Operations

### Preparing Files for Transfer

1. **Place files in the root directory**:
   ```bash
   sudo cp firmware.bin /var/tftp/
   sudo chmod 644 /var/tftp/firmware.bin
   ```

2. **Organize by subdirectories**:
   ```bash
   sudo mkdir -p /var/tftp/firmware
   sudo mkdir -p /var/tftp/configs
   sudo cp firmware.bin /var/tftp/firmware/
   ```

### Using TFTP Client

#### Download a File (Read)
```bash
# Using tftp command
tftp localhost
tftp> get firmware.bin
tftp> quit

# Or in one command
tftp localhost -c get firmware.bin
```

#### Upload a File (Write)
```bash
# Enable write in configuration first
# Then upload
tftp localhost
tftp> put localfile.bin remotefile.bin
tftp> quit
```

### File Permissions

Ensure files are readable:
```bash
# Make files readable
sudo chmod 644 /var/tftp/*

# Make directories readable and executable
sudo chmod 755 /var/tftp/
```

## Security Basics

### Directory Restrictions

Limit access to specific directories:
```json
{
  "filesystem": {
    "root_directory": "/var/tftp",
    "allowed_directories": [
      "/var/tftp/firmware",
      "/var/tftp/configs"
    ]
  }
}
```

### File Extension Filtering

Allow only specific file types:
```json
{
  "security": {
    "allowed_extensions": [".bin", ".img", ".cfg", ".txt"],
    "blocked_extensions": [".exe", ".sh", ".bat"]
  }
}
```

### Client IP Filtering

Restrict access to specific IP ranges:
```json
{
  "security": {
    "allowed_clients": [
      "192.168.1.0/24",    // Local network
      "10.0.0.0/8"         // Private network
    ]
  }
}
```

### Read-Only Mode

For maximum security, disable writes:
```json
{
  "security": {
    "read_enabled": true,
    "write_enabled": false
  }
}
```

## Common Use Cases

### Use Case 1: PXE Boot Server

```json
{
  "filesystem": {
    "root_directory": "/var/tftp",
    "allowed_directories": ["/var/tftp/pxe"]
  },
  "security": {
    "read_enabled": true,
    "write_enabled": false,
    "allowed_extensions": [".pxe", ".bin", ".cfg"]
  }
}
```

### Use Case 2: Firmware Distribution

```json
{
  "filesystem": {
    "root_directory": "/var/tftp/firmware"
  },
  "security": {
    "read_enabled": true,
    "write_enabled": false,
    "allowed_extensions": [".bin", ".img", ".fw"],
    "max_file_size": 52428800  // 50 MB
  }
}
```

### Use Case 3: Configuration File Distribution

```json
{
  "filesystem": {
    "root_directory": "/var/tftp/configs"
  },
  "security": {
    "read_enabled": true,
    "write_enabled": true,
    "allowed_extensions": [".cfg", ".conf", ".ini"],
    "overwrite_protection": true
  }
}
```

### Use Case 4: Development/Testing

```json
{
  "filesystem": {
    "root_directory": "/tmp/tftp",
    "create_directories": true
  },
  "security": {
    "read_enabled": true,
    "write_enabled": true,
    "overwrite_protection": false
  },
  "logging": {
    "level": "DEBUG",
    "console_logging": true
  }
}
```

## Configuration Reload

Reload configuration without restarting:

### Systemd
```bash
sudo systemctl reload simple-tftpd
```

### Manual (SIGHUP)
```bash
# Find process ID
ps aux | grep simple-tftpd

# Send SIGHUP
sudo kill -HUP <PID>
```

## Monitoring

### Check Server Status
```bash
# Systemd
sudo systemctl status simple-tftpd

# Manual check
simple-tftpd status
```

### View Logs
```bash
# System logs
sudo journalctl -u simple-tftpd -f

# Log file
tail -f /var/log/simple-tftpd.log
```

### Test Configuration
```bash
# Validate configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

## Performance Tuning

### Block Size
```json
{
  "performance": {
    "block_size": 512  // Standard TFTP block size
  }
}
```

### Window Size
```json
{
  "performance": {
    "window_size": 8  // Increase for better throughput
  }
}
```

### Timeout
```json
{
  "performance": {
    "timeout": 5  // Seconds
  }
}
```

## Next Steps

- **[Configuration Reference](../configuration/README.md)** - Complete configuration guide
- **[Production Deployment](../../production/deployment.md)** - Production deployment
- **[Security Best Practices](../user-guide/security-best-practices.md)** - Security guide
- **[Troubleshooting](../troubleshooting/README.md)** - Common issues

## Additional Resources

- **[Examples](../examples/README.md)** - Configuration examples
- **[User Guide](../user-guide/README.md)** - User documentation
- **[Architecture Diagrams](../diagrams/architecture.md)** - System architecture

---

**Last Updated**: December 2024
