# Quick Start Guide

Get Simple TFTP Daemon running in minutes!

## Prerequisites

- Linux, macOS, or Windows
- Root/Administrator access (for port 69)
- Basic command-line knowledge

## Step 1: Installation

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install simple-tftpd
```

### macOS
```bash
brew install simple-tftpd
```

### From Source
```bash
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Step 2: Create Configuration

```bash
# Create configuration directory
sudo mkdir -p /etc/simple-tftpd

# Copy example configuration
sudo cp config/simple-tftpd.conf.example /etc/simple-tftpd/simple-tftpd.conf
```

## Step 3: Create TFTP Root Directory

```bash
# Create TFTP root directory
sudo mkdir -p /var/tftp

# Set permissions (optional, adjust as needed)
sudo chmod 755 /var/tftp
```

## Step 4: Basic Configuration

Edit `/etc/simple-tftpd/simple-tftpd.conf`:

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
    "console_logging": true
  }
}
```

## Step 5: Start the Server

### Manual Start
```bash
sudo simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
```

### Systemd Service (Linux)
```bash
# Create service file
sudo tee /etc/systemd/system/simple-tftpd.service > /dev/null <<EOF
[Unit]
Description=Simple TFTP Daemon
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# Enable and start
sudo systemctl daemon-reload
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd
sudo systemctl status simple-tftpd
```

## Step 6: Test the Server

### Test File Transfer

1. **Create a test file**:
   ```bash
   echo "Hello, TFTP!" > /var/tftp/test.txt
   ```

2. **Test read (download)**:
   ```bash
   # Using tftp client
   tftp localhost
   tftp> get test.txt
   tftp> quit
   ```

3. **Test write (upload)** (if enabled):
   ```bash
   # Using tftp client
   tftp localhost
   tftp> put test.txt uploaded.txt
   tftp> quit
   ```

### Verify Server Status

```bash
# Check if server is running
sudo systemctl status simple-tftpd

# Check logs
sudo tail -f /var/log/simple-tftpd.log

# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

## Common Configuration Options

### Enable Write Operations
```json
{
  "security": {
    "write_enabled": true,
    "overwrite_protection": true
  }
}
```

### Restrict to Specific Directory
```json
{
  "filesystem": {
    "root_directory": "/var/tftp",
    "allowed_directories": ["/var/tftp/firmware", "/var/tftp/configs"]
  }
}
```

### Filter by File Extension
```json
{
  "security": {
    "allowed_extensions": [".bin", ".img", ".cfg"],
    "blocked_extensions": [".exe", ".sh"]
  }
}
```

### Client IP Filtering
```json
{
  "security": {
    "allowed_clients": ["192.168.1.0/24", "10.0.0.0/8"]
  }
}
```

## Next Steps

- **[First Steps](first-steps.md)** - Learn more about configuration
- **[Configuration Reference](../configuration/README.md)** - Complete configuration guide
- **[Production Deployment](../../production/deployment.md)** - Production deployment
- **[Troubleshooting](../troubleshooting/README.md)** - Common issues

## Troubleshooting

### Port Already in Use
```bash
# Check what's using port 69
sudo netstat -tulpn | grep :69

# Stop conflicting service
sudo systemctl stop tftpd
```

### Permission Denied
```bash
# Check directory permissions
ls -la /var/tftp

# Fix permissions
sudo chmod 755 /var/tftp
sudo chown tftp:tftp /var/tftp  # If tftp user exists
```

### Server Won't Start
```bash
# Check logs
sudo journalctl -u simple-tftpd -n 50

# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

For more troubleshooting help, see the [Troubleshooting Guide](../troubleshooting/README.md).

---

**Last Updated**: December 2024
