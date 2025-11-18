# simple-tftpd Deployment

This directory contains deployment configurations and examples for simple-tftpd.

## Directory Structure

```
deployment/
├── systemd/                    # Linux systemd service files
│   └── simple-tftpd.service
├── launchd/                    # macOS launchd service files
│   └── com.simpledaemons.simple-tftpd.plist
├── logrotate.d/                # Linux log rotation configuration
│   └── simple-tftpd
├── windows/                    # Windows service management
│   └── simple-tftpd.service.bat
└── examples/                   # Deployment examples
    └── docker/                 # Docker deployment examples
        ├── docker-compose.yml
        └── README.md
```

## Platform-Specific Deployment

### Linux (systemd)

1. **Install the service file:**
   ```bash
   sudo cp deployment/systemd/simple-tftpd.service /etc/systemd/system/
   sudo systemctl daemon-reload
   ```

2. **Create user and group:**
   ```bash
   sudo useradd --system --no-create-home --shell /bin/false simple-tftpd
   ```

3. **Enable and start the service:**
   ```bash
   sudo systemctl enable simple-tftpd
   sudo systemctl start simple-tftpd
   ```

4. **Check status:**
   ```bash
   sudo systemctl status simple-tftpd
   sudo journalctl -u simple-tftpd -f
   ```

### macOS (launchd)

1. **Install the plist file:**
   ```bash
   sudo cp deployment/launchd/com.simpledaemons.simple-tftpd.plist /Library/LaunchDaemons/
   sudo chown root:wheel /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist
   ```

2. **Load and start the service:**
   ```bash
   sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist
   sudo launchctl start com.simpledaemons.simple-tftpd
   ```

3. **Check status:**
   ```bash
   sudo launchctl list | grep simple-tftpd
   tail -f /var/log/simple-tftpd.log
   ```

### Windows

1. **Run as Administrator:**
   ```cmd
   # Install service
   deployment\windows\simple-tftpd.service.bat install
   
   # Start service
   deployment\windows\simple-tftpd.service.bat start
   
   # Check status
   deployment\windows\simple-tftpd.service.bat status
   ```

2. **Service management:**
   ```cmd
   # Stop service
   deployment\windows\simple-tftpd.service.bat stop
   
   # Restart service
   deployment\windows\simple-tftpd.service.bat restart
   
   # Uninstall service
   deployment\windows\simple-tftpd.service.bat uninstall
   ```

## Log Rotation (Linux)

1. **Install logrotate configuration:**
   ```bash
   sudo cp deployment/logrotate.d/simple-tftpd /etc/logrotate.d/
   ```

2. **Test logrotate configuration:**
   ```bash
   sudo logrotate -d /etc/logrotate.d/simple-tftpd
   ```

3. **Force log rotation:**
   ```bash
   sudo logrotate -f /etc/logrotate.d/simple-tftpd
   ```

## Docker Deployment

See [examples/docker/README.md](examples/docker/README.md) for detailed Docker deployment instructions.

### Quick Start

```bash
# Build and run with Docker Compose
cd deployment/examples/docker
docker-compose up -d

# Check status
docker-compose ps
docker-compose logs simple-tftpd
```

## Configuration

### Service Configuration

Each platform has specific configuration requirements:

- **Linux**: Edit `/etc/systemd/system/simple-tftpd.service`
- **macOS**: Edit `/Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist`
- **Windows**: Edit the service binary path in the batch file

### Application Configuration

Place your application configuration in:
- **Linux/macOS**: `/etc/simple-tftpd/simple-tftpd.conf`
- **Windows**: `%PROGRAMFILES%\simple-tftpd\simple-tftpd.conf`

## Security Considerations

### User and Permissions

1. **Create dedicated user:**
   ```bash
   # Linux
   sudo useradd --system --no-create-home --shell /bin/false simple-tftpd
   
   # macOS
   sudo dscl . -create /Users/_simple-tftpd UserShell /usr/bin/false
   sudo dscl . -create /Users/_simple-tftpd UniqueID 200
   sudo dscl . -create /Users/_simple-tftpd PrimaryGroupID 200
   sudo dscl . -create /Groups/_simple-tftpd GroupID 200
   ```

2. **Set proper permissions:**
   ```bash
   # Configuration files
   sudo chown root:simple-tftpd /etc/simple-tftpd/simple-tftpd.conf
   sudo chmod 640 /etc/simple-tftpd/simple-tftpd.conf
   
   # Log files
   sudo chown simple-tftpd:simple-tftpd /var/log/simple-tftpd/
   sudo chmod 755 /var/log/simple-tftpd/
   ```

### Firewall Configuration

Configure firewall rules as needed:

```bash
# Linux (ufw)
sudo ufw allow 69/tcp

# Linux (firewalld)
sudo firewall-cmd --permanent --add-port=69/tcp
sudo firewall-cmd --reload

# macOS
sudo pfctl -f /etc/pf.conf
```

## Monitoring

### Health Checks

1. **Service status:**
   ```bash
   # Linux
   sudo systemctl is-active simple-tftpd
   
   # macOS
   sudo launchctl list | grep simple-tftpd
   
   # Windows
   sc query simple-tftpd
   ```

2. **Port availability:**
   ```bash
   netstat -tlnp | grep 69
   ss -tlnp | grep 69
   ```

3. **Process monitoring:**
   ```bash
   ps aux | grep simple-tftpd
   top -p $(pgrep simple-tftpd)
   ```

### Log Monitoring

1. **Real-time logs:**
   ```bash
   # Linux
   sudo journalctl -u simple-tftpd -f
   
   # macOS
   tail -f /var/log/simple-tftpd.log
   
   # Windows
   # Use Event Viewer or PowerShell Get-WinEvent
   ```

2. **Log analysis:**
   ```bash
   # Search for errors
   sudo journalctl -u simple-tftpd --since "1 hour ago" | grep -i error
   
   # Count log entries
   sudo journalctl -u simple-tftpd --since "1 day ago" | wc -l
   ```

## Troubleshooting

### Common Issues

1. **Service won't start:**
   - Check configuration file syntax
   - Verify user permissions
   - Check port availability
   - Review service logs

2. **Permission denied:**
   - Ensure service user exists
   - Check file permissions
   - Verify directory ownership

3. **Port already in use:**
   - Check what's using the port: `netstat -tlnp | grep 69`
   - Stop conflicting service or change port

4. **Service stops unexpectedly:**
   - Check application logs
   - Verify resource limits
   - Review system logs

### Debug Mode

Run the service in debug mode for troubleshooting:

```bash
# Linux/macOS
sudo -u simple-tftpd /usr/local/bin/simple-tftpd --debug

# Windows
simple-tftpd.exe --debug
```

### Log Levels

Adjust log level for more verbose output:

```bash
# Set log level in configuration
log_level = debug

# Or via environment variable
export SIMPLE-TFTPD_LOG_LEVEL=debug
```

## Backup and Recovery

### Configuration Backup

```bash
# Backup configuration
sudo tar -czf simple-tftpd-config-backup-$(date +%Y%m%d).tar.gz /etc/simple-tftpd/

# Backup logs
sudo tar -czf simple-tftpd-logs-backup-$(date +%Y%m%d).tar.gz /var/log/simple-tftpd/
```

### Service Recovery

```bash
# Stop service
sudo systemctl stop simple-tftpd

# Restore configuration
sudo tar -xzf simple-tftpd-config-backup-YYYYMMDD.tar.gz -C /

# Start service
sudo systemctl start simple-tftpd
```

## Updates

### Service Update Process

1. **Stop service:**
   ```bash
   sudo systemctl stop simple-tftpd
   ```

2. **Backup current version:**
   ```bash
   sudo cp /usr/local/bin/simple-tftpd /usr/local/bin/simple-tftpd.backup
   ```

3. **Install new version:**
   ```bash
   sudo cp simple-tftpd /usr/local/bin/
   sudo chmod +x /usr/local/bin/simple-tftpd
   ```

4. **Start service:**
   ```bash
   sudo systemctl start simple-tftpd
   ```

5. **Verify update:**
   ```bash
   sudo systemctl status simple-tftpd
   simple-tftpd --version
   ```
