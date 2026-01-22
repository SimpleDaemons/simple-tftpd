# Production Version Configuration Guide

**Version:** 0.2.0-beta
**License:** Apache 2.0

---

## Overview

This guide covers configuration for the Production Version of Simple TFTP Daemon. The Production Version supports JSON, YAML, and INI configuration formats.

## Configuration File Location

Default configuration file locations:
- **Linux:** `/etc/simple-tftpd/simple-tftpd.conf`
- **macOS:** `/usr/local/etc/simple-tftpd/simple-tftpd.conf`
- **Windows:** `C:\Program Files\Simple TFTP Daemon\config\simple-tftpd.conf`

## Basic Configuration

### Minimal Configuration

```json
{
  "tftp": {
    "listen_address": "0.0.0.0",
    "port": 69,
    "root_directory": "/var/tftpboot"
  }
}
```

### Complete Configuration Example

```json
{
  "tftp": {
    "listen_address": "0.0.0.0",
    "port": 69,
    "root_directory": "/var/tftpboot",
    "allowed_directories": ["/var/tftpboot"],
    "allowed_extensions": [".bin", ".img", ".cfg", ".txt"],
    "max_file_size": 104857600,
    "window_size": 8,
    "block_size": 512,
    "timeout": 5,
    "max_retries": 3,
    "read_enabled": true,
    "write_enabled": false,
    "overwrite_protection": true,
    "ip_filtering": {
      "enabled": true,
      "allowed_ips": ["192.168.1.0/24"]
    },
    "logging": {
      "level": "INFO",
      "file": "/var/log/simple-tftpd/simple-tftpd.log"
    }
  }
}
```

## Configuration Reference

### TFTP Server Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `listen_address` | string | `"0.0.0.0"` | IP address to listen on |
| `port` | integer | `69` | UDP port to listen on |
| `root_directory` | string | `"/var/tftpboot"` | Root directory for file operations |

### Security Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `allowed_directories` | array | `[]` | List of allowed directories |
| `allowed_extensions` | array | `[]` | List of allowed file extensions |
| `max_file_size` | integer | `0` | Maximum file size (0 = unlimited) |
| `read_enabled` | boolean | `true` | Enable read operations |
| `write_enabled` | boolean | `false` | Enable write operations |
| `overwrite_protection` | boolean | `true` | Prevent file overwrites |

### Transfer Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `window_size` | integer | `1` | Window size for windowed transfers |
| `block_size` | integer | `512` | Default block size |
| `timeout` | integer | `5` | Timeout in seconds |
| `max_retries` | integer | `3` | Maximum retry attempts |

### IP Filtering

```json
{
  "tftp": {
    "ip_filtering": {
      "enabled": true,
      "allowed_ips": ["192.168.1.0/24", "10.0.0.0/8"],
      "denied_ips": ["192.168.1.100"]
    }
  }
}
```

### Logging Configuration

```json
{
  "tftp": {
    "logging": {
      "level": "INFO",
      "format": "STANDARD",
      "file": "/var/log/simple-tftpd/simple-tftpd.log",
      "console": true
    }
  }
}
```

## Advanced Configuration

### Multiple Root Directories

```json
{
  "tftp": {
    "root_directory": "/var/tftpboot",
    "allowed_directories": [
      "/var/tftpboot/firmware",
      "/var/tftpboot/configs",
      "/var/tftpboot/images"
    ]
  }
}
```

### Transfer Mode Configuration

```json
{
  "tftp": {
    "transfer_modes": {
      "netascii": {
        "enabled": true,
        "crlf_conversion": true
      },
      "octet": {
        "enabled": true
      },
      "mail": {
        "enabled": false
      }
    }
  }
}
```

## Configuration Validation

### Validate Configuration

```bash
# Test configuration file
simple-tftpd -c /etc/simple-tftpd/simple-tftpd.conf --test-config

# Check for errors
simple-tftpd -c /etc/simple-tftpd/simple-tftpd.conf --validate
```

### Common Configuration Errors

1. **Invalid Directory Paths**
   - Ensure all directories exist
   - Check directory permissions
   - Verify path format

2. **Invalid IP Addresses**
   - Ensure CIDR notation is correct
   - Check IP address format

3. **Invalid File Extensions**
   - Use dot notation (`.bin` not `bin`)
   - Check for typos

## Hot Reloading

The Production Version supports hot reloading via SIGHUP:

```bash
# Reload configuration
sudo killall -HUP simple-tftpd

# Or using systemd
sudo systemctl reload simple-tftpd
```

## Configuration Examples

See [Configuration Examples](../shared/configuration/examples.md) for more examples.

## Related Documentation

- [Installation Guide](../shared/getting-started/README.md)
- [Deployment Guide](deployment.md)
- [Security Guide](security.md)
- [Performance Tuning](performance.md)

---

**Last Updated:** December 2024
**Version:** 0.2.0-beta
