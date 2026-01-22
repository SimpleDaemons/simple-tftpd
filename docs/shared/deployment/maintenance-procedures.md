# Maintenance Procedures

This guide covers maintenance procedures for Simple TFTP Daemon.

## Overview

Regular maintenance ensures:
- Optimal performance
- Security updates
- System stability
- Data integrity

## Regular Maintenance Tasks

### Daily Tasks

- [ ] Check server status
- [ ] Review error logs
- [ ] Monitor disk space
- [ ] Check backup status

### Weekly Tasks

- [ ] Review performance metrics
- [ ] Check log file sizes
- [ ] Verify backup integrity
- [ ] Review security alerts

### Monthly Tasks

- [ ] Update software (if needed)
- [ ] Review and rotate logs
- [ ] Check disk usage
- [ ] Review configuration
- [ ] Test disaster recovery

### Quarterly Tasks

- [ ] Security audit
- [ ] Performance review
- [ ] Capacity planning
- [ ] Documentation review

## Server Status Checks

### Check Service Status

```bash
# Systemd
sudo systemctl status simple-tftpd

# Manual check
simple-tftpd status

# Health check
simple-tftpd health
```

### Check Logs

```bash
# Recent logs
sudo journalctl -u simple-tftpd -n 50

# Error logs
sudo journalctl -u simple-tftpd -p err

# Log file
tail -f /var/log/simple-tftpd.log
```

## Log Management

### Log Rotation

Create `/etc/logrotate.d/simple-tftpd`:

```
/var/log/simple-tftpd/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0640 tftp tftp
    sharedscripts
    postrotate
        systemctl reload simple-tftpd > /dev/null 2>&1 || true
    endscript
}
```

### Manual Log Rotation

```bash
# Rotate logs
sudo logrotate -f /etc/logrotate.d/simple-tftpd

# Or manually
sudo mv /var/log/simple-tftpd/simple-tftpd.log /var/log/simple-tftpd/simple-tftpd.log.old
sudo systemctl reload simple-tftpd
```

## Configuration Management

### Configuration Review

```bash
# Validate configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check for deprecated options
simple-tftpd config-check --config /etc/simple-tftpd/simple-tftpd.conf
```

### Configuration Backup

```bash
# Before making changes
sudo cp /etc/simple-tftpd/simple-tftpd.conf /etc/simple-tftpd/simple-tftpd.conf.backup

# After changes
sudo systemctl reload simple-tftpd
```

## Software Updates

### Check for Updates

```bash
# Package manager
sudo apt update
apt list --upgradable | grep simple-tftpd

# Or check version
simple-tftpd --version
```

### Update Procedure

```bash
# 1. Backup configuration
sudo cp -r /etc/simple-tftpd /var/backups/simple-tftpd/config-$(date +%Y%m%d)

# 2. Stop service
sudo systemctl stop simple-tftpd

# 3. Update software
sudo apt update && sudo apt upgrade simple-tftpd

# 4. Verify configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# 5. Start service
sudo systemctl start simple-tftpd

# 6. Verify service
sudo systemctl status simple-tftpd
```

## Disk Space Management

### Check Disk Usage

```bash
# Overall disk usage
df -h

# TFTP directory usage
du -sh /var/tftp

# Log directory usage
du -sh /var/log/simple-tftpd
```

### Cleanup Old Files

```bash
# Remove old log files
find /var/log/simple-tftpd -name "*.log.*" -mtime +30 -delete

# Remove old backup files
find /var/backups/simple-tftpd -type f -mtime +90 -delete

# Clean temporary files
find /var/tftp -name "*.tmp" -delete
```

## Performance Tuning

### Monitor Performance

```bash
# Check resource usage
top -p $(pgrep simple-tftpd)

# Check connections
simple-tftpd metrics | grep connections

# Check response time
simple-tftpd metrics | grep response_time
```

### Performance Optimization

```json
{
  "performance": {
    "block_size": 512,
    "window_size": 8,
    "timeout": 5,
    "max_connections": 1000
  }
}
```

## Security Maintenance

### Security Updates

```bash
# Update system packages
sudo apt update && sudo apt upgrade

# Check for security advisories
# Monitor: https://github.com/SimpleDaemons/simple-tftpd/security
```

### Security Audit

```bash
# Check file permissions
find /var/tftp -type f ! -perm 644 -ls
find /var/tftp -type d ! -perm 755 -ls

# Check configuration security
simple-tftpd security-audit --config /etc/simple-tftpd/simple-tftpd.conf
```

### Firewall Review

```bash
# Check firewall rules
sudo firewall-cmd --list-all

# Review allowed clients
grep allowed_clients /etc/simple-tftpd/simple-tftpd.conf
```

## Database Maintenance (Enterprise/Datacenter)

### PostgreSQL Maintenance

```bash
# Vacuum database
sudo -u postgres vacuumdb -d simple_tftpd

# Analyze database
sudo -u postgres psql -d simple_tftpd -c "ANALYZE;"

# Check database size
sudo -u postgres psql -d simple_tftpd -c "SELECT pg_size_pretty(pg_database_size('simple_tftpd'));"
```

### MySQL Maintenance

```bash
# Optimize tables
mysqlcheck -u simple_tftpd -p --optimize simple_tftpd

# Analyze tables
mysqlcheck -u simple_tftpd -p --analyze simple_tftpd
```

## Cluster Maintenance (Enterprise/Datacenter)

### Cluster Health Check

```bash
# Check cluster status
simple-tftpd-enterprise cluster-status

# Check node health
simple-tftpd-enterprise node-health

# Check cluster logs
simple-tftpd-enterprise cluster-logs
```

### Rolling Updates

```bash
# Update one node at a time
# 1. Drain node
simple-tftpd-enterprise node-drain node-1

# 2. Update node
sudo apt upgrade simple-tftpd-enterprise

# 3. Restart node
sudo systemctl restart simple-tftpd-enterprise

# 4. Verify node
simple-tftpd-enterprise node-health node-1

# 5. Re-enable node
simple-tftpd-enterprise node-uncordon node-1
```

## Monitoring Maintenance

### Review Metrics

```bash
# Check metrics endpoint
curl http://localhost:8081/api/v1/metrics

# Review Grafana dashboards
# Check for anomalies or trends
```

### Alert Review

- Review alert history
- Tune alert thresholds
- Update alert recipients
- Test alert delivery

## Documentation Maintenance

### Update Documentation

- Keep documentation current
- Document configuration changes
- Update runbooks
- Review procedures

## Maintenance Windows

### Schedule Maintenance

```bash
# Schedule maintenance window
# Notify users
# Perform maintenance
# Verify service
# Close maintenance window
```

### Maintenance Checklist

- [ ] Notify users
- [ ] Backup configuration
- [ ] Backup data
- [ ] Perform maintenance
- [ ] Verify service
- [ ] Update documentation
- [ ] Notify completion

## Troubleshooting During Maintenance

### Common Issues

**Service Won't Start:**
```bash
# Check logs
sudo journalctl -u simple-tftpd -n 50

# Check configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check permissions
ls -la /var/tftp
```

**Performance Degradation:**
```bash
# Check resource usage
top -p $(pgrep simple-tftpd)

# Check connections
simple-tftpd metrics

# Review logs for errors
sudo journalctl -u simple-tftpd -p err
```

## Best Practices

1. **Regular Maintenance**: Schedule regular maintenance windows
2. **Documentation**: Document all maintenance activities
3. **Backup First**: Always backup before maintenance
4. **Test Changes**: Test changes in non-production first
5. **Monitor**: Monitor after maintenance
6. **Rollback Plan**: Have a rollback plan ready
7. **Communication**: Notify users of maintenance windows

## Next Steps

- **[Backup Procedures](backup-procedures.md)** - Backup and restore
- **[Monitoring](monitoring.md)** - Monitoring setup
- **[Troubleshooting](../../troubleshooting/README.md)** - Common issues

---

**Last Updated**: December 2024
