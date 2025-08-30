# Simple TFTP Daemon

A lightweight, cross-platform TFTP (Trivial File Transfer Protocol) daemon written in C++.

## Features

- **Cross-platform**: Supports Linux, macOS, and Windows
- **RFC Compliant**: Implements TFTP protocol according to RFC 1350
- **Modern C++**: Built with C++17 standard
- **Configurable**: JSON-based configuration with command-line overrides
- **Secure**: Configurable access controls and file restrictions
- **Logging**: Comprehensive logging with configurable levels
- **IPv6 Support**: Optional IPv6 support
- **Performance**: Configurable block sizes and timeouts
- **Statistics**: Built-in connection and transfer statistics

## License

Copyright 2024 SimpleDaemons

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- jsoncpp library
- Platform-specific development libraries

### Build Commands

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

- `BUILD_TESTS`: Build test suite (default: ON)
- `BUILD_EXAMPLES`: Build examples (default: OFF)
- `ENABLE_LOGGING`: Enable logging (default: ON)
- `ENABLE_IPV6`: Enable IPv6 support (default: ON)

## Usage

### Basic Usage

```bash
# Start server with default settings
./simple-tftpd start

# Start server with custom configuration
./simple-tftpd start --config /etc/simple-tftpd/config.json

# Start server with command-line options
./simple-tftpd start --listen 0.0.0.0 --port 69 --root /var/tftp
```

### Command Line Options

- `--help, -h`: Show help message
- `--version, -v`: Show version information
- `--config, -c FILE`: Use specified configuration file
- `--verbose, -V`: Enable verbose logging
- `--daemon, -d`: Run as daemon
- `--foreground, -f`: Run in foreground
- `--listen ADDR`: Listen on specific address
- `--port PORT`: Listen on specific port
- `--root DIR`: Set root directory for file operations

### Commands

- `start`: Start the TFTP server
- `stop`: Stop the TFTP server
- `restart`: Restart the TFTP server
- `status`: Show server status
- `reload`: Reload configuration
- `test`: Test server configuration
- `stats`: Show server statistics
- `connections`: List active connections

## Configuration

The daemon can be configured using a JSON configuration file or command-line options.

### Configuration File Example

```json
{
    "network": {
        "listen_address": "0.0.0.0",
        "listen_port": 69,
        "ipv6_enabled": true
    },
    "filesystem": {
        "root_directory": "/var/tftp",
        "allowed_directories": ["/var/tftp/public"]
    },
    "security": {
        "read_enabled": true,
        "write_enabled": false,
        "max_file_size": 104857600
    },
    "performance": {
        "block_size": 512,
        "timeout": 5
    },
    "logging": {
        "level": "INFO",
        "log_file": "/var/log/simple-tftpd.log"
    }
}
```

## TFTP Protocol Support

- **Read Request (RRQ)**: Client downloads files from server
- **Write Request (WRQ)**: Client uploads files to server
- **Data Packets**: File data transfer
- **Acknowledgment Packets**: Transfer confirmation
- **Error Packets**: Error reporting
- **Transfer Modes**: netascii, octet, mail

## Security Features

- Configurable read/write permissions
- File size limits
- Directory access restrictions
- Overwrite protection
- Client address validation

## Performance Features

- Configurable block sizes (default: 512 bytes)
- Adjustable timeouts
- Windowed transfer support
- Connection pooling
- Efficient memory management

## Logging

The daemon provides comprehensive logging with configurable levels:

- **DEBUG**: Detailed debugging information
- **INFO**: General information messages
- **WARNING**: Warning messages
- **ERROR**: Error messages
- **FATAL**: Fatal error messages

## Examples

### Basic TFTP Client Usage

```bash
# Download a file
tftp localhost 69
tftp> get filename.txt

# Upload a file (if write enabled)
tftp localhost 69
tftp> put filename.txt
```

### Systemd Service

Create `/etc/systemd/system/simple-tftpd.service`:

```ini
[Unit]
Description=Simple TFTP Daemon
After=network.target

[Service]
Type=simple
User=tftp
ExecStart=/usr/bin/simple-tftpd start --config /etc/simple-tftpd/config.json
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

## Development

### Project Structure

```
simple-tftpd/
├── cmake/                 # CMake configuration files
├── config/                # Configuration files
├── docs/                  # Documentation
├── include/               # Header files
│   └── simple_tftpd/
├── scripts/               # Build and utility scripts
├── src/                   # Source files
│   ├── core/             # Core TFTP functionality
│   ├── utils/            # Utility classes
│   ├── tests/            # Test suite
│   └── examples/         # Example programs
├── tools/                 # Development tools
├── CMakeLists.txt         # Main CMake file
└── README.md             # This file
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## Testing

Run the test suite:

```bash
cd build
make test
```

## Troubleshooting

### Common Issues

1. **Permission Denied**: Ensure the daemon has access to the root directory
2. **Port Already in Use**: Check if port 69 is available or use a different port
3. **File Not Found**: Verify the root directory exists and contains the requested files
4. **Connection Refused**: Check firewall settings and ensure the daemon is running

### Debug Mode

Enable debug logging for troubleshooting:

```bash
./simple-tftpd start --verbose
```

## Roadmap

- [ ] TFTP options support (blksize, timeout, tsize, windowsize)
- [ ] Multicast TFTP support
- [ ] Authentication and authorization
- [ ] SSL/TLS support
- [ ] Web-based management interface
- [ ] SNMP monitoring
- [ ] Rate limiting
- [ ] Advanced logging (syslog, structured logging)

## Support

For issues and questions:

1. Check the documentation
2. Search existing issues
3. Create a new issue with detailed information

## Acknowledgments

- RFC 1350: The TFTP Protocol
- RFC 2347: TFTP Option Extension
- RFC 2348: TFTP Blocksize Option
- RFC 2349: TFTP Timeout Interval and Transfer Size Options
