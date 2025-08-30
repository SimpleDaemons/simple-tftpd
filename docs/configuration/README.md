# Configuration Reference

This document provides a complete reference for all configuration options available in the Simple TFTP Daemon.

## Table of Contents

- [Configuration File Format](#configuration-file-format)
- [Configuration Options](#configuration-options)
- [Environment Variables](#environment-variables)
- [Command Line Overrides](#command-line-overrides)
- [Configuration Validation](#configuration-validation)
- [Examples](#examples)
- [Best Practices](#best-practices)

## Configuration File Format

The Simple TFTP Daemon uses JSON configuration files. The configuration is hierarchical and organized into logical sections.

### Basic Structure

```json
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
        "log_file": "",
        "console_logging": true
    }
}
```

### File Location

The daemon looks for configuration files in the following order:

1. Command line specified file (`--config` option)
2. Environment variable `SIMPLE_TFTPD_CONFIG_FILE`
3. Platform-specific default locations:
   - **Linux**: `/etc/simple-tftpd/simple-tftpd.conf`
   - **macOS**: `/etc/simple-tftpd/simple-tftpd.conf`
   - **Windows**: `C:\ProgramData\simple-tftpd\simple-tftpd.conf`
4. Current working directory: `./simple-tftpd.conf`

## Configuration Options

### Network Configuration

#### `network.listen_address`

- **Type**: string
- **Default**: "0.0.0.0"
- **Description**: IP address to bind the server to
- **Values**:
  - `"0.0.0.0"` - Listen on all IPv4 interfaces
  - `"::"` - Listen on all IPv6 interfaces
  - `"127.0.0.1"` - Listen only on localhost
  - `"192.168.1.100"` - Listen on specific interface

**Example**:
```json
{
    "network": {
        "listen_address": "192.168.1.100"
    }
}
```

#### `network.listen_port`

- **Type**: integer
- **Default**: 69
- **Range**: 1-65535
- **Description**: Port number to listen on
- **Note**: Ports below 1024 require root privileges on Unix systems

**Example**:
```json
{
    "network": {
        "listen_port": 6969
    }
}
```

#### `network.ipv6_enabled`

- **Type**: boolean
- **Default**: true
- **Description**: Enable IPv6 support
- **Note**: Requires IPv6 support in the operating system

**Example**:
```json
{
    "network": {
        "ipv6_enabled": false
    }
}
```

### File System Configuration

#### `filesystem.root_directory`

- **Type**: string
- **Default**: "/var/tftp"
- **Description**: Root directory for all TFTP operations
- **Note**: Must be an absolute path and accessible by the daemon

**Example**:
```json
{
    "filesystem": {
        "root_directory": "/home/user/tftp-files"
    }
}
```

#### `filesystem.allowed_directories`

- **Type**: array of strings
- **Default**: []
- **Description**: List of directories allowed for file operations
- **Note**: Empty array means all subdirectories of root are allowed

**Example**:
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

#### `filesystem.create_directories`

- **Type**: boolean
- **Default**: false
- **Description**: Automatically create directories if they don't exist
- **Note**: Only applies to write operations

**Example**:
```json
{
    "filesystem": {
        "create_directories": true
    }
}
```

### Security Configuration

#### `security.read_enabled`

- **Type**: boolean
- **Default**: true
- **Description**: Allow clients to download files
- **Note**: Disabling this prevents all read operations

**Example**:
```json
{
    "security": {
        "read_enabled": false
    }
}
```

#### `security.write_enabled`

- **Type**: boolean
- **Default**: false
- **Description**: Allow clients to upload files
- **Note**: Enabling this may pose security risks

**Example**:
```json
{
    "security": {
        "write_enabled": true
    }
}
```

#### `security.max_file_size`

- **Type**: integer
- **Default**: 104857600 (100MB)
- **Range**: 1-2147483647
- **Description**: Maximum file size in bytes
- **Note**: Applies to both read and write operations

**Example**:
```json
{
    "security": {
        "max_file_size": 52428800
    }
}
```

#### `security.overwrite_protection`

- **Type**: boolean
- **Default**: true
- **Description**: Prevent overwriting existing files
- **Note**: Only applies to write operations

**Example**:
```json
{
    "security": {
        "overwrite_protection": false
    }
}
```

#### `security.allowed_extensions`

- **Type**: array of strings
- **Default**: []
- **Description**: List of allowed file extensions for write operations
- **Note**: Empty array means all extensions are allowed

**Example**:
```json
{
    "security": {
        "allowed_extensions": [
            ".txt",
            ".bin",
            ".img",
            ".cfg"
        ]
    }
}
```

#### `security.blocked_extensions`

- **Type**: array of strings
- **Default**: []
- **Description**: List of blocked file extensions
- **Note**: Takes precedence over allowed extensions

**Example**:
```json
{
    "security": {
        "blocked_extensions": [
            ".exe",
            ".bat",
            ".sh"
        ]
    }
}
```

### Performance Configuration

#### `performance.block_size`

- **Type**: integer
- **Default**: 512
- **Range**: 8-65464
- **Description**: TFTP block size in bytes
- **Note**: Must be a multiple of 8, larger sizes improve performance

**Example**:
```json
{
    "performance": {
        "block_size": 1024
    }
}
```

#### `performance.timeout`

- **Type**: integer
- **Default**: 5
- **Range**: 1-255
- **Description**: Timeout in seconds for packet acknowledgment
- **Note**: Lower values may cause timeouts on slow networks

**Example**:
```json
{
    "performance": {
        "timeout": 10
    }
}
```

#### `performance.window_size`

- **Type**: integer
- **Default**: 1
- **Range**: 1-65535
- **Description**: Window size for block transfers
- **Note**: Larger values improve performance but may not be supported by all clients

**Example**:
```json
{
    "performance": {
        "window_size": 4
    }
}
```

#### `performance.max_connections`

- **Type**: integer
- **Default**: 100
- **Range**: 1-10000
- **Description**: Maximum number of concurrent connections
- **Note**: Higher values increase memory usage

**Example**:
```json
{
    "performance": {
        "max_connections": 50
    }
}
```

#### `performance.connection_timeout`

- **Type**: integer
- **Default**: 300
- **Range**: 1-3600
- **Description**: Connection timeout in seconds
- **Note**: Inactive connections are closed after this time

**Example**:
```json
{
    "performance": {
        "connection_timeout": 600
    }
}
```

### Logging Configuration

#### `logging.level`

- **Type**: string
- **Default**: "INFO"
- **Values**: "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"
- **Description**: Minimum log level to output
- **Note**: DEBUG provides the most detailed output

**Example**:
```json
{
    "logging": {
        "level": "DEBUG"
    }
}
```

#### `logging.log_file`

- **Type**: string
- **Default**: ""
- **Description**: Path to log file
- **Note**: Empty string disables file logging

**Example**:
```json
{
    "logging": {
        "log_file": "/var/log/simple-tftpd.log"
    }
}
```

#### `logging.console_logging`

- **Type**: boolean
- **Default**: true
- **Description**: Enable logging to console
- **Note**: Useful for debugging and foreground operation

**Example**:
```json
{
    "logging": {
        "console_logging": false
    }
}
```

#### `logging.max_file_size`

- **Type**: integer
- **Default**: 10485760 (10MB)
- **Range**: 1-1073741824
- **Description**: Maximum log file size before rotation
- **Note**: Only applies when log_file is specified

**Example**:
```json
{
    "logging": {
        "max_file_size": 52428800
    }
}
```

#### `logging.max_files`

- **Type**: integer
- **Default**: 5
- **Range**: 1-100
- **Description**: Maximum number of log files to keep
- **Note**: Old log files are automatically removed

**Example**:
```json
{
    "logging": {
        "max_files": 10
    }
}
```

## Environment Variables

You can override configuration values using environment variables. Environment variables take precedence over configuration file values.

### Variable Naming

Environment variables use the format: `SIMPLE_TFTPD_SECTION_OPTION`

### Examples

```bash
# Network settings
export SIMPLE_TFTPD_NETWORK_LISTEN_ADDRESS=192.168.1.100
export SIMPLE_TFTPD_NETWORK_LISTEN_PORT=6969
export SIMPLE_TFTPD_NETWORK_IPV6_ENABLED=false

# File system settings
export SIMPLE_TFTPD_FILESYSTEM_ROOT_DIRECTORY=/home/user/tftp
export SIMPLE_TFTPD_FILESYSTEM_CREATE_DIRECTORIES=true

# Security settings
export SIMPLE_TFTPD_SECURITY_WRITE_ENABLED=true
export SIMPLE_TFTPD_SECURITY_MAX_FILE_SIZE=52428800

# Performance settings
export SIMPLE_TFTPD_PERFORMANCE_BLOCK_SIZE=1024
export SIMPLE_TFTPD_PERFORMANCE_TIMEOUT=10

# Logging settings
export SIMPLE_TFTPD_LOGGING_LEVEL=DEBUG
export SIMPLE_TFTPD_LOGGING_LOG_FILE=/tmp/tftp.log
```

## Command Line Overrides

Command line options take precedence over both environment variables and configuration file values.

### Available Options

```bash
# Network options
--listen ADDR          # Override listen_address
--port PORT            # Override listen_port
--no-ipv6             # Disable IPv6

# File system options
--root DIR             # Override root_directory
--create-dirs          # Enable directory creation

# Security options
--read-only            # Disable write operations
--write-allowed        # Enable write operations
--max-size SIZE        # Override max_file_size

# Performance options
--block-size SIZE      # Override block_size
--timeout SECONDS      # Override timeout
--max-connections NUM  # Override max_connections

# Logging options
--log-level LEVEL      # Override log level
--log-file FILE        # Override log file
--no-console           # Disable console logging
```

### Examples

```bash
# Start with custom network settings
simple-tftpd start --listen 192.168.1.100 --port 6969

# Start with custom file system settings
simple-tftpd start --root /home/user/tftp --create-dirs

# Start with custom security settings
simple-tftpd start --write-allowed --max-size 50MB

# Start with custom performance settings
simple-tftpd start --block-size 1024 --timeout 10

# Start with custom logging
simple-tftpd start --log-level DEBUG --log-file /tmp/tftp.log
```

## Configuration Validation

### Validation Rules

The daemon validates configuration values according to these rules:

1. **Type Checking**: Values must match expected types
2. **Range Validation**: Numeric values must be within specified ranges
3. **Path Validation**: File paths must be accessible
4. **Dependency Checking**: Related options must be consistent

### Validation Commands

```bash
# Test configuration file
simple-tftpd test

# Validate specific configuration
simple-tftpd test --config /path/to/config.json

# Check configuration with verbose output
simple-tftpd test --verbose
```

### Common Validation Errors

#### Type Errors

```json
{
    "network": {
        "listen_port": "69"  // Error: expected integer
    }
}
```

**Fix**:
```json
{
    "network": {
        "listen_port": 69
    }
}
```

#### Range Errors

```json
{
    "performance": {
        "block_size": 10000  // Error: exceeds maximum 65464
    }
}
```

**Fix**:
```json
{
    "performance": {
        "block_size": 8192
    }
}
```

#### Path Errors

```json
{
    "filesystem": {
        "root_directory": "relative/path"  // Error: must be absolute
    }
}
```

**Fix**:
```json
{
    "filesystem": {
        "root_directory": "/absolute/path"
    }
}
```

## Examples

### Minimal Configuration

```json
{
    "network": {
        "listen_address": "0.0.0.0",
        "listen_port": 69
    },
    "filesystem": {
        "root_directory": "/var/tftp"
    }
}
```

### Development Configuration

```json
{
    "network": {
        "listen_address": "127.0.0.1",
        "listen_port": 6969,
        "ipv6_enabled": false
    },
    "filesystem": {
        "root_directory": "/tmp/tftp-dev",
        "create_directories": true
    },
    "security": {
        "read_enabled": true,
        "write_enabled": true,
        "max_file_size": 1048576
    },
    "logging": {
        "level": "DEBUG",
        "console_logging": true
    }
}
```

### Production Configuration

```json
{
    "network": {
        "listen_address": "192.168.1.100",
        "listen_port": 69,
        "ipv6_enabled": true
    },
    "filesystem": {
        "root_directory": "/var/tftp",
        "allowed_directories": [
            "/var/tftp/firmware",
            "/var/tftp/configs"
        ]
    },
    "security": {
        "read_enabled": true,
        "write_enabled": false,
        "max_file_size": 104857600,
        "overwrite_protection": true
    },
    "performance": {
        "block_size": 1024,
        "timeout": 10,
        "max_connections": 200
    },
    "logging": {
        "level": "INFO",
        "log_file": "/var/log/simple-tftpd.log",
        "console_logging": false,
        "max_file_size": 10485760,
        "max_files": 10
    }
}
```

### High-Security Configuration

```json
{
    "network": {
        "listen_address": "10.0.0.100",
        "listen_port": 6969,
        "ipv6_enabled": false
    },
    "filesystem": {
        "root_directory": "/var/tftp/secure",
        "allowed_directories": [
            "/var/tftp/secure/readonly"
        ]
    },
    "security": {
        "read_enabled": true,
        "write_enabled": false,
        "max_file_size": 1048576,
        "overwrite_protection": true,
        "allowed_extensions": [
            ".txt",
            ".cfg"
        ]
    },
    "performance": {
        "block_size": 512,
        "timeout": 5,
        "max_connections": 50
    },
    "logging": {
        "level": "WARNING",
        "log_file": "/var/log/simple-tftpd-secure.log",
        "console_logging": false
    }
}
```

## Best Practices

### Security

1. **Restrict Network Access**: Use specific IP addresses instead of 0.0.0.0
2. **Limit File Operations**: Disable write operations unless necessary
3. **Set File Size Limits**: Prevent abuse through large file uploads
4. **Use Allowed Directories**: Restrict access to specific directories
5. **Enable Overwrite Protection**: Prevent accidental file overwrites

### Performance

1. **Optimize Block Size**: Use larger block sizes for better performance
2. **Set Appropriate Timeouts**: Balance between reliability and performance
3. **Limit Connections**: Set reasonable connection limits
4. **Monitor Resource Usage**: Watch memory and CPU usage

### Logging

1. **Use Appropriate Log Levels**: INFO for production, DEBUG for development
2. **Enable File Logging**: For production environments
3. **Implement Log Rotation**: Prevent log files from growing too large
4. **Monitor Log Files**: Watch for errors and unusual activity

### File System

1. **Use Absolute Paths**: Ensure consistent behavior across environments
2. **Set Proper Permissions**: Restrict access to configuration and log files
3. **Organize Content**: Use subdirectories for different types of files
4. **Regular Maintenance**: Clean up old and unused files

### Configuration Management

1. **Use Version Control**: Track configuration changes
2. **Environment-Specific Configs**: Different configs for dev/staging/prod
3. **Document Changes**: Keep track of configuration modifications
4. **Test Configurations**: Validate before deploying to production

## Troubleshooting

### Common Configuration Issues

#### Server Won't Start

**Problem**: Invalid configuration file
```bash
simple-tftpd: error: Invalid configuration file
```

**Solution**: Validate configuration
```bash
simple-tftpd test --config /path/to/config.json
```

**Problem**: Permission denied
```bash
simple-tftpd: error: Cannot access root directory
```

**Solution**: Check directory permissions
```bash
sudo chown tftp:tftp /var/tftp
sudo chmod 755 /var/tftp
```

#### Configuration Not Applied

**Problem**: Environment variables not recognized
```bash
simple-tftpd: warning: Unknown configuration option
```

**Solution**: Check environment variable naming
```bash
# Correct format
export SIMPLE_TFTPD_NETWORK_LISTEN_ADDRESS=192.168.1.100

# Incorrect format
export SIMPLE_TFTPD_LISTEN_ADDRESS=192.168.1.100
```

### Getting Help

- **Check Logs**: Look for configuration-related error messages
- **Validate Configuration**: Use `simple-tftpd test` command
- **Check Documentation**: Refer to this configuration reference
- **Report Issues**: Create GitHub issue with configuration details

## Next Steps

After configuring the daemon:

1. **[User Guide](../user-guide/README.md)** - Learn how to use the service
2. **[Examples](../examples/README.md)** - See practical configuration examples
3. **[Development Guide](../development/README.md)** - Contribute to the project
