# Backup and Restore Procedures

This guide covers backup and restore procedures for Simple TFTP Daemon.

## Overview

Regular backups are essential for:
- Disaster recovery
- Configuration recovery
- Data protection
- Compliance requirements

## What to Backup

### Configuration Files

- `/etc/simple-tftpd/simple-tftpd.conf` - Main configuration
- `/etc/simple-tftpd/*.conf` - Additional configuration files
- Custom configuration files

### Data Files

- `/var/tftp/*` - TFTP root directory and all files
- `/var/lib/simple-tftpd/*` - Application data (Enterprise/Datacenter)

### Logs (Optional)

- `/var/log/simple-tftpd/*` - Log files
- System logs related to Simple TFTP Daemon

### Database (Enterprise/Datacenter)

- PostgreSQL database
- MySQL database
- Other database backends

## Backup Methods

### Manual Backup

#### Configuration Backup

```bash
# Create backup directory
sudo mkdir -p /var/backups/simple-tftpd

# Backup configuration
sudo cp -r /etc/simple-tftpd /var/backups/simple-tftpd/config-$(date +%Y%m%d)

# Compress
sudo tar -czf /var/backups/simple-tftpd/config-$(date +%Y%m%d).tar.gz /etc/simple-tftpd
```

#### Data Backup

```bash
# Backup TFTP root directory
sudo tar -czf /var/backups/simple-tftpd/data-$(date +%Y%m%d).tar.gz /var/tftp

# Or use rsync
sudo rsync -av /var/tftp/ /var/backups/simple-tftpd/data-$(date +%Y%m%d)/
```

#### Database Backup (Enterprise/Datacenter)

```bash
# PostgreSQL
pg_dump -U simple_tftpd simple_tftpd > /var/backups/simple-tftpd/db-$(date +%Y%m%d).sql

# MySQL
mysqldump -u simple_tftpd -p simple_tftpd > /var/backups/simple-tftpd/db-$(date +%Y%m%d).sql
```

### Automated Backup Script

Create `/usr/local/bin/backup-simple-tftpd.sh`:

```bash
#!/bin/bash

BACKUP_DIR="/var/backups/simple-tftpd"
DATE=$(date +%Y%m%d-%H%M%S)
RETENTION_DAYS=30

# Create backup directory
mkdir -p $BACKUP_DIR

# Backup configuration
tar -czf $BACKUP_DIR/config-$DATE.tar.gz /etc/simple-tftpd

# Backup data
tar -czf $BACKUP_DIR/data-$DATE.tar.gz /var/tftp

# Backup database (if Enterprise/Datacenter)
if command -v pg_dump &> /dev/null; then
    pg_dump -U simple_tftpd simple_tftpd > $BACKUP_DIR/db-$DATE.sql
fi

# Remove old backups
find $BACKUP_DIR -type f -mtime +$RETENTION_DAYS -delete

echo "Backup completed: $DATE"
```

Make executable:
```bash
sudo chmod +x /usr/local/bin/backup-simple-tftpd.sh
```

### Cron Job

Add to crontab:
```bash
# Daily backup at 2 AM
0 2 * * * /usr/local/bin/backup-simple-tftpd.sh
```

## Backup Configuration

### Built-in Backup (Enterprise/Datacenter)

```json
{
  "backup": {
    "enabled": true,
    "schedule": "0 2 * * *",
    "retention_days": 30,
    "location": "/var/backups/simple-tftpd",
    "compress": true,
    "encrypt": false
  }
}
```

### Cloud Backup

**AWS S3:**
```bash
# Install AWS CLI
sudo apt install awscli

# Backup to S3
aws s3 sync /var/backups/simple-tftpd s3://my-backup-bucket/simple-tftpd/
```

**Azure Blob Storage:**
```bash
# Install Azure CLI
curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

# Backup to Azure
az storage blob upload-batch \
  --destination mycontainer \
  --source /var/backups/simple-tftpd \
  --account-name mystorageaccount
```

**Google Cloud Storage:**
```bash
# Install gsutil
# Backup to GCS
gsutil -m cp -r /var/backups/simple-tftpd gs://my-backup-bucket/
```

## Restore Procedures

### Configuration Restore

```bash
# Stop service
sudo systemctl stop simple-tftpd

# Restore configuration
sudo tar -xzf /var/backups/simple-tftpd/config-20241201.tar.gz -C /

# Verify configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Start service
sudo systemctl start simple-tftpd
```

### Data Restore

```bash
# Stop service
sudo systemctl stop simple-tftpd

# Restore data
sudo tar -xzf /var/backups/simple-tftpd/data-20241201.tar.gz -C /

# Set permissions
sudo chown -R tftp:tftp /var/tftp

# Start service
sudo systemctl start simple-tftpd
```

### Database Restore (Enterprise/Datacenter)

```bash
# Stop service
sudo systemctl stop simple-tftpd-enterprise

# Restore database
# PostgreSQL
psql -U simple_tftpd simple_tftpd < /var/backups/simple-tftpd/db-20241201.sql

# MySQL
mysql -u simple_tftpd -p simple_tftpd < /var/backups/simple-tftpd/db-20241201.sql

# Start service
sudo systemctl start simple-tftpd-enterprise
```

### Full System Restore

```bash
# Stop service
sudo systemctl stop simple-tftpd

# Restore configuration
sudo tar -xzf /var/backups/simple-tftpd/config-20241201.tar.gz -C /

# Restore data
sudo tar -xzf /var/backups/simple-tftpd/data-20241201.tar.gz -C /

# Restore database (if applicable)
psql -U simple_tftpd simple_tftpd < /var/backups/simple-tftpd/db-20241201.sql

# Verify
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Start service
sudo systemctl start simple-tftpd
```

## Backup Verification

### Verify Backup Integrity

```bash
# Check backup file
tar -tzf /var/backups/simple-tftpd/config-20241201.tar.gz

# Test restore to temporary location
mkdir /tmp/test-restore
tar -xzf /var/backups/simple-tftpd/config-20241201.tar.gz -C /tmp/test-restore
```

### Regular Testing

Test restore procedures regularly:
1. Restore to test environment
2. Verify data integrity
3. Test service functionality
4. Document any issues

## Disaster Recovery

### Recovery Plan

1. **Assess Damage**: Determine what needs to be restored
2. **Stop Services**: Stop affected services
3. **Restore Configuration**: Restore configuration files
4. **Restore Data**: Restore data files
5. **Restore Database**: Restore database (if applicable)
6. **Verify**: Test service functionality
7. **Start Services**: Start services
8. **Monitor**: Monitor for issues

### Recovery Time Objectives (RTO)

- **Configuration**: < 5 minutes
- **Data**: < 15 minutes
- **Full System**: < 30 minutes

### Recovery Point Objectives (RPO)

- **Configuration**: Last backup (daily)
- **Data**: Last backup (daily)
- **Database**: Last backup (hourly for Enterprise/Datacenter)

## Best Practices

1. **Regular Backups**: Daily backups minimum
2. **Offsite Storage**: Keep backups offsite
3. **Encryption**: Encrypt sensitive backups
4. **Verification**: Regularly verify backup integrity
5. **Documentation**: Document backup and restore procedures
6. **Testing**: Regularly test restore procedures
7. **Retention**: Keep backups for appropriate retention period
8. **Monitoring**: Monitor backup success/failure

## Next Steps

- **[Maintenance Procedures](maintenance-procedures.md)** - Maintenance tasks
- **[Monitoring](monitoring.md)** - Monitoring setup
- **[Troubleshooting](../../troubleshooting/README.md)** - Common issues

---

**Last Updated**: December 2024
