# Production Version Security Guide

**Version:** 0.2.0-beta
**License:** Apache 2.0

---

## Overview

This guide covers security features and best practices for the Production Version of Simple TFTP Daemon.

## Security Features

### Directory Access Control

Restrict file access to specific directories:

```json
{
  "tftp": {
    "root_directory": "/var/tftpboot",
    "allowed_directories": [
      "/var/tftpboot/firmware",
      "/var/tftpboot/configs"
    ]
  }
}
```

### File Extension Filtering

Allow only specific file types:

```json
{
  "tftp": {
    "allowed_extensions": [".bin", ".img", ".cfg", ".txt"]
  }
}
```

### IP Address Filtering

Restrict access by client IP:

```json
{
  "tftp": {
    "ip_filtering": {
      "enabled": true,
      "allowed_ips": ["192.168.1.0/24", "10.0.0.0/8"]
    }
  }
}
```

### File Size Limits

Prevent large file transfers:

```json
{
  "tftp": {
    "max_file_size": 104857600
  }
}
```

### Read/Write Permissions

Control read and write access:

```json
{
  "tftp": {
    "read_enabled": true,
    "write_enabled": false
  }
}
```

### Overwrite Protection

Prevent accidental file overwrites:

```json
{
  "tftp": {
    "overwrite_protection": true
  }
}
```

## Security Best Practices

1. **Use Directory Allowlists** - Restrict access to specific directories
2. **Enable IP Filtering** - Limit access to trusted networks
3. **Disable Write by Default** - Only enable write when needed
4. **Set File Size Limits** - Prevent abuse
5. **Use Overwrite Protection** - Prevent accidental data loss
6. **Monitor Logs** - Regularly review security logs
7. **Keep Updated** - Keep software updated

See [Security Best Practices](../shared/user-guide/security-best-practices.md) for detailed guidelines.

---

**Last Updated:** December 2024
**Version:** 0.2.0-beta
