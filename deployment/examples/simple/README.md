# Simple Setup Examples

This directory contains simple, single-instance deployment examples.

## Quick Start

### 1. Install the Package
```bash
sudo apt install simple-tftpd  # Ubuntu/Debian
sudo yum install simple-tftpd  # CentOS/RHEL
make install                    # From source
```

### 2. Create Basic Configuration
```bash
sudo mkdir -p /etc/simple-tftpd
sudo cp /usr/share/simple-tftpd/simple-tftpd.conf.example /etc/simple-tftpd/simple-tftpd.conf
sudo mkdir -p /var/tftp
sudo chown tftp:tftp /var/tftp
```

### 3. Start the Service
```bash
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd
sudo systemctl status simple-tftpd
```

### 4. Test the Service
```bash
tftp localhost 69
tftp> get testfile.txt
tftp> quit
```

## Basic Configuration

```json
{
    "network": {
        "listen_address": "0.0.0.0",
        "listen_port": 69
    },
    "filesystem": {
        "root_directory": "/var/tftp"
    },
    "security": {
        "read_enabled": true,
        "write_enabled": false
    }
}
```

## Service Management

```bash
# Start/Stop/Status
sudo systemctl start simple-tftpd
sudo systemctl stop simple-tftpd
sudo systemctl restart simple-tftpd
sudo systemctl status simple-tftpd

# View logs
sudo journalctl -u simple-tftpd -f

# Test configuration
sudo simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

## Troubleshooting

1. **Port already in use**: `sudo netstat -tulpn | grep :69`
2. **Permission denied**: `sudo chown -R tftp:tftp /var/tftp`
3. **Service won't start**: Check logs with `journalctl -u simple-tftpd`
