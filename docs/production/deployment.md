# Production Version Deployment Guide

**Version:** 0.2.0-beta
**License:** Apache 2.0

---

## Overview

This guide covers deploying the Production Version of Simple TFTP Daemon in production environments.

## Pre-Deployment Checklist

- [ ] System requirements met
- [ ] Network configuration planned
- [ ] Root directory created and configured
- [ ] Configuration file prepared
- [ ] Firewall rules configured
- [ ] Monitoring setup planned
- [ ] Backup strategy defined

## Deployment Methods

### Package Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install simple-tftpd
```

**CentOS/RHEL:**
```bash
sudo yum install simple-tftpd
```

**macOS:**
```bash
brew install simple-tftpd
```

### Docker Deployment

```bash
docker run -d \
  --name simple-tftpd \
  --net host \
  -v /var/tftpboot:/var/tftpboot \
  -v /etc/simple-tftpd:/etc/simple-tftpd \
  -v /var/log/simple-tftpd:/var/log/simple-tftpd \
  simpledaemons/simple-tftpd:production
```

### Systemd Service

```bash
# Enable and start service
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd

# Check status
sudo systemctl status simple-tftpd
```

## Configuration

1. **Create Configuration Directory**
```bash
sudo mkdir -p /etc/simple-tftpd
sudo chown tftpd:tftpd /etc/simple-tftpd
```

2. **Create Root Directory**
```bash
sudo mkdir -p /var/tftpboot
sudo chown tftpd:tftpd /var/tftpboot
sudo chmod 755 /var/tftpboot
```

3. **Create Configuration File**
```bash
sudo nano /etc/simple-tftpd/simple-tftpd.conf
```

4. **Validate Configuration**
```bash
sudo simple-tftpd -c /etc/simple-tftpd/simple-tftpd.conf --test-config
```

## Network Configuration

### Firewall Rules

**UFW (Ubuntu):**
```bash
sudo ufw allow 69/udp
```

**firewalld (CentOS/RHEL):**
```bash
sudo firewall-cmd --permanent --add-service=tftp
sudo firewall-cmd --reload
```

### Network Interface

Ensure the TFTP server can bind to the correct interface:
```bash
# Check interface
ip addr show

# Configure listen address in config
"listen_address": "192.168.1.10"
```

## Verification

### Test TFTP Server

```bash
# Start server
sudo systemctl start simple-tftpd

# Check logs
sudo journalctl -u simple-tftpd -f

# Test from client
tftp 192.168.1.10
tftp> get testfile.txt
tftp> quit
```

## Monitoring

See [Monitoring Guide](../shared/deployment/monitoring.md) for monitoring setup.

## Backup

See [Backup Procedures](../shared/deployment/backup-procedures.md) for backup configuration.

## Maintenance

See [Maintenance Procedures](../shared/deployment/maintenance-procedures.md) for maintenance tasks.

## Troubleshooting

See [Troubleshooting Guide](../shared/troubleshooting/README.md) for common issues.

---

**Last Updated:** December 2024
**Version:** 0.2.0-beta
