# Examples Guide

This guide provides practical examples of how to use the Simple TFTP Daemon.

## Basic Examples

### Starting the Server

```bash
# Start with default settings
simple-tftpd start

# Start with custom port
simple-tftpd start --port 6969

# Start with configuration file
simple-tftpd start --config /etc/simple-tftpd/config.json
```

### Testing Connections

```bash
# Test file download
tftp localhost 69
tftp> get testfile.txt
tftp> quit
```

## Configuration Examples

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
        "listen_port": 6969
    },
    "filesystem": {
        "root_directory": "/tmp/tftp-dev"
    },
    "logging": {
        "level": "DEBUG"
    }
}
```

## Next Steps

1. **[User Guide](../user-guide/README.md)** - Learn more about using the daemon
2. **[Configuration Reference](../configuration/README.md)** - Customize the configuration
3. **[Development Guide](../development/README.md)** - Contribute to the project
