# Simple TFTP Daemon - Production Deployment Guide

**Version:** v0.2.0-beta  
**Last Updated:** December 2024

---

## Table of Contents

1. [System Requirements](#system-requirements)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Service Management](#service-management)
5. [Security Hardening](#security-hardening)
6. [Monitoring & Health Checks](#monitoring--health-checks)
7. [Performance Tuning](#performance-tuning)
8. [Troubleshooting](#troubleshooting)
9. [Backup & Recovery](#backup--recovery)

---

## System Requirements

### Minimum Requirements

- **CPU:** 1 core (2+ recommended)
- **RAM:** 256 MB (512 MB+ recommended)
- **Disk:** 100 MB for installation + space for TFTP files
- **Network:** UDP port 69 (or custom port)

### Supported Platforms

- **Linux:** Ubuntu 20.04+, Debian 11+, CentOS 8+, RHEL 8+
- **macOS:** 10.15+ (Catalina and later)
- **Windows:** Windows 10/11, Windows Server 2016+

### Dependencies

- **Runtime:**
  - OpenSSL 1.1.1+ (for SSL/TLS support)
  - jsoncpp (for configuration parsing)
  - libc++ (C++17 standard library)

- **Build (if compiling from source):**
  - CMake 3.16+
  - C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
  - Make or Ninja

---

## Installation

### Package Installation

#### Ubuntu/Debian

```bash
# Install from DEB package
sudo dpkg -i simple-tftpd_0.2.0-beta_amd64.deb
sudo apt-get install -f  # Install dependencies if needed
```

#### CentOS/RHEL

```bash
# Install from RPM package
sudo rpm -ivh simple-tftpd-0.2.0-beta.x86_64.rpm
```

#### macOS

```bash
# Install from DMG
open simple-tftpd-0.2.0-beta.dmg
# Drag to Applications folder

# Or install from PKG
sudo installer -pkg simple-tftpd-0.2.0-beta.pkg -target /
```

### Docker Installation

```bash
# Pull image
docker pull simpledaemons/simple-tftpd:0.2.0-beta

# Run container
docker run -d \
  --name simple-tftpd \
  -p 69:69/udp \
  -v /var/tftp:/var/tftp:ro \
  -v /etc/simple-tftpd:/etc/simple-tftpd:ro \
  simpledaemons/simple-tftpd:0.2.0-beta
```

### Source Installation

```bash
# Clone repository
git clone https://github.com/simpledaemons/simple-tftpd.git
cd simple-tftpd

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install
sudo make install
```

---

## Configuration

### Configuration File Location

- **Linux:** `/etc/simple-tftpd/simple-tftpd.conf`
- **macOS:** `/usr/local/etc/simple-tftpd/simple-tftpd.conf`
- **Windows:** `C:\Program Files\simple-tftpd\config\simple-tftpd.conf`

### Basic Configuration

Create `/etc/simple-tftpd/simple-tftpd.conf`:

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
        "max_file_size": 104857600,
        "overwrite_protection": true,
        "allowed_extensions": ["bin", "img", "txt"],
        "allowed_clients": []
    },
    "performance": {
        "block_size": 512,
        "timeout": 5,
        "window_size": 1,
        "max_retries": 5
    },
    "logging": {
        "level": "INFO",
        "log_file": "/var/log/simple-tftpd.log",
        "console_logging": false
    }
}
```

### Configuration Validation

```bash
# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Validate and show errors
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf --verbose
```

### Hot Reload

Reload configuration without restarting the server:

```bash
# Send SIGHUP signal
sudo killall -HUP simple-tftpd

# Or use systemd
sudo systemctl reload simple-tftpd
```

---

## Service Management

### systemd (Linux)

#### Service File

Create `/etc/systemd/system/simple-tftpd.service`:

```ini
[Unit]
Description=Simple TFTP Daemon
After=network.target

[Service]
Type=simple
User=tftp
Group=tftp
ExecStart=/usr/bin/simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

#### Service Commands

```bash
# Start service
sudo systemctl start simple-tftpd

# Stop service
sudo systemctl stop simple-tftpd

# Restart service
sudo systemctl restart simple-tftpd

# Reload configuration
sudo systemctl reload simple-tftpd

# Enable auto-start on boot
sudo systemctl enable simple-tftpd

# Check status
sudo systemctl status simple-tftpd

# View logs
sudo journalctl -u simple-tftpd -f
```

### launchd (macOS)

#### Plist File

Create `/Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.simpledaemons.simple-tftpd</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/simple-tftpd</string>
        <string>start</string>
        <string>--config</string>
        <string>/usr/local/etc/simple-tftpd/simple-tftpd.conf</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>/var/log/simple-tftpd.log</string>
    <key>StandardErrorPath</key>
    <string>/var/log/simple-tftpd.error.log</string>
</dict>
</plist>
```

#### Service Commands

```bash
# Load service
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist

# Unload service
sudo launchctl unload /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist

# Start service
sudo launchctl start com.simpledaemons.simple-tftpd

# Stop service
sudo launchctl stop com.simpledaemons.simple-tftpd
```

### Windows Service

Install as Windows Service:

```powershell
# Install service
sc create simple-tftpd binPath="C:\Program Files\simple-tftpd\simple-tftpd.exe start --config C:\Program Files\simple-tftpd\config\simple-tftpd.conf"

# Start service
sc start simple-tftpd

# Stop service
sc stop simple-tftpd

# Delete service
sc delete simple-tftpd
```

---

## Security Hardening

### 1. Run as Non-Privileged User

```bash
# Create dedicated user
sudo useradd -r -s /bin/false -d /var/tftp tftp

# Set ownership
sudo chown -R tftp:tftp /var/tftp
sudo chown -R tftp:tftp /var/log/simple-tftpd.log
```

### 2. File System Permissions

```bash
# Restrict root directory
sudo chmod 755 /var/tftp
sudo chmod 644 /var/tftp/*

# Prevent directory traversal
sudo chmod 755 /var/tftp/public
sudo chmod 644 /var/tftp/public/*
```

### 3. Network Security

```json
{
    "security": {
        "allowed_clients": [
            "192.168.1.0/24",
            "10.0.0.0/8"
        ],
        "read_enabled": true,
        "write_enabled": false
    }
}
```

### 4. Firewall Configuration

#### Linux (iptables)

```bash
# Allow TFTP from specific network
sudo iptables -A INPUT -p udp --dport 69 -s 192.168.1.0/24 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 69 -j DROP
```

#### Linux (firewalld)

```bash
# Allow TFTP from specific zone
sudo firewall-cmd --permanent --add-service=tftp
sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" source address="192.168.1.0/24" service name="tftp" accept'
sudo firewall-cmd --reload
```

### 5. File Size Limits

```json
{
    "security": {
        "max_file_size": 10485760  // 10 MB limit
    }
}
```

### 6. Extension Filtering

```json
{
    "security": {
        "allowed_extensions": ["bin", "img", "cfg"]
    }
}
```

---

## Monitoring & Health Checks

### Health Check API

The server provides health check functionality:

```bash
# Check server health (via status command)
simple-tftpd status

# Get health check JSON
simple-tftpd status --json
```

### Metrics Collection

```bash
# Get server metrics
simple-tftpd stats

# Get metrics as JSON
simple-tftpd stats --json
```

### Monitoring Integration

#### Prometheus (Future)

```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'simple-tftpd'
    static_configs:
      - targets: ['localhost:9090']
```

#### Log Monitoring

```bash
# Monitor logs in real-time
tail -f /var/log/simple-tftpd.log

# Search for errors
grep ERROR /var/log/simple-tftpd.log

# Count errors
grep -c ERROR /var/log/simple-tftpd.log
```

### Health Check Script

Create `/usr/local/bin/check-tftpd-health.sh`:

```bash
#!/bin/bash
HEALTH=$(simple-tftpd status --json | jq -r '.status')
if [ "$HEALTH" != "healthy" ]; then
    echo "TFTP server is unhealthy: $HEALTH"
    exit 1
fi
exit 0
```

---

## Performance Tuning

### Block Size Optimization

For high-speed networks, increase block size:

```json
{
    "performance": {
        "block_size": 1024  // or 1400 for MTU-optimized
    }
}
```

### Window Size Tuning

For better throughput on reliable networks:

```json
{
    "performance": {
        "window_size": 4  // or higher for very reliable networks
    }
}
```

### Timeout Configuration

Adjust based on network latency:

```json
{
    "performance": {
        "timeout": 10,  // Increase for high-latency networks
        "max_retries": 5
    }
}
```

### System Limits

```bash
# Increase file descriptor limits
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# Apply immediately
ulimit -n 65536
```

---

## Troubleshooting

### Common Issues

#### 1. Permission Denied

**Problem:** Server cannot access files

**Solution:**
```bash
# Check file permissions
ls -la /var/tftp

# Fix ownership
sudo chown -R tftp:tftp /var/tftp
```

#### 2. Port Already in Use

**Problem:** Port 69 is already in use

**Solution:**
```bash
# Check what's using the port
sudo netstat -ulnp | grep :69
sudo lsof -i :69

# Use different port in config
{
    "network": {
        "listen_port": 6969
    }
}
```

#### 3. Connection Refused

**Problem:** Clients cannot connect

**Solution:**
- Check firewall rules
- Verify server is running: `systemctl status simple-tftpd`
- Check listen address (0.0.0.0 for all interfaces)
- Verify network connectivity

#### 4. File Not Found

**Problem:** Files not accessible

**Solution:**
- Check root directory path
- Verify file exists and is readable
- Check allowed_directories configuration
- Verify file extension is allowed

#### 5. Transfer Timeout

**Problem:** Transfers timeout frequently

**Solution:**
- Increase timeout value
- Check network stability
- Increase max_retries
- Verify block_size is appropriate

### Debug Mode

Enable debug logging:

```json
{
    "logging": {
        "level": "DEBUG"
    }
}
```

Or via command line:

```bash
simple-tftpd start --verbose --config /etc/simple-tftpd/simple-tftpd.conf
```

### Log Analysis

```bash
# View recent errors
grep ERROR /var/log/simple-tftpd.log | tail -20

# Count errors by type
grep ERROR /var/log/simple-tftpd.log | awk '{print $5}' | sort | uniq -c

# Monitor connections
grep "Connection" /var/log/simple-tftpd.log | tail -20
```

---

## Backup & Recovery

### Configuration Backup

```bash
# Backup configuration
sudo cp /etc/simple-tftpd/simple-tftpd.conf /etc/simple-tftpd/simple-tftpd.conf.backup

# Backup with timestamp
sudo cp /etc/simple-tftpd/simple-tftpd.conf \
       /etc/simple-tftpd/simple-tftpd.conf.$(date +%Y%m%d)
```

### TFTP Files Backup

```bash
# Backup TFTP root directory
sudo tar -czf /backup/tftp-$(date +%Y%m%d).tar.gz /var/tftp

# Restore from backup
sudo tar -xzf /backup/tftp-20241217.tar.gz -C /
```

### Automated Backup Script

```bash
#!/bin/bash
# /usr/local/bin/backup-tftpd.sh

BACKUP_DIR="/backup/tftpd"
DATE=$(date +%Y%m%d)

mkdir -p $BACKUP_DIR

# Backup configuration
cp /etc/simple-tftpd/simple-tftpd.conf $BACKUP_DIR/config-$DATE.conf

# Backup TFTP files
tar -czf $BACKUP_DIR/files-$DATE.tar.gz /var/tftp

# Keep only last 7 days
find $BACKUP_DIR -name "*.tar.gz" -mtime +7 -delete
find $BACKUP_DIR -name "*.conf" -mtime +7 -delete
```

---

## Production Checklist

Before deploying to production:

- [ ] Configuration file created and validated
- [ ] Service user created (non-root)
- [ ] File permissions set correctly
- [ ] Firewall rules configured
- [ ] Logging configured
- [ ] Monitoring set up
- [ ] Backup strategy in place
- [ ] Health checks tested
- [ ] Performance tested under load
- [ ] Security audit completed
- [ ] Documentation reviewed

---

## Support

For production support:

- **Documentation:** See `docs/` directory
- **Issues:** https://github.com/simpledaemons/simple-tftpd/issues
- **Logs:** Check `/var/log/simple-tftpd.log`
- **Status:** Run `simple-tftpd status`

---

*Last Updated: December 2024*
