# Production Version Operations Guide

**Version:** 0.2.0-beta
**License:** Apache 2.0

---

## Overview

This guide covers day-to-day operations for the Production Version of Simple TFTP Daemon.

## Service Management

### Start/Stop Service

```bash
# Start
sudo systemctl start simple-tftpd

# Stop
sudo systemctl stop simple-tftpd

# Restart
sudo systemctl restart simple-tftpd

# Reload configuration
sudo systemctl reload simple-tftpd
```

### Check Status

```bash
# Service status
sudo systemctl status simple-tftpd

# Check logs
sudo journalctl -u simple-tftpd -f
```

## Configuration Management

### Reload Configuration

```bash
# Reload without restart
sudo systemctl reload simple-tftpd

# Or send SIGHUP
sudo killall -HUP simple-tftpd
```

### Validate Configuration

```bash
sudo simple-tftpd -c /etc/simple-tftpd/simple-tftpd.conf --test-config
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
