# Security Best Practices

This guide covers security best practices for Simple TFTP Daemon.

## Overview

Security is critical for TFTP servers. This guide provides best practices for securing your Simple TFTP Daemon deployment.

## Network Security

### Firewall Configuration

**Restrict Access:**
```bash
# Allow only specific networks
sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" source address="192.168.1.0/24" port port="69" protocol="udp" accept'

# Block all other access
sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" port port="69" protocol="udp" drop'
```

**Configuration:**
```json
{
  "security": {
    "allowed_clients": [
      "192.168.1.0/24",
      "10.0.0.0/8"
    ]
  }
}
```

### Network Isolation

- Use VLANs to isolate TFTP traffic
- Deploy behind a firewall
- Use VPN for remote access
- Consider network segmentation

## File System Security

### Directory Permissions

```bash
# Set proper permissions
sudo chmod 755 /var/tftp
sudo chown tftp:tftp /var/tftp

# Restrict file access
sudo chmod 644 /var/tftp/*
```

### Path Traversal Protection

```json
{
  "security": {
    "path_traversal_protection": true,
    "allowed_directories": [
      "/var/tftp/firmware",
      "/var/tftp/configs"
    ]
  }
}
```

### File Extension Filtering

```json
{
  "security": {
    "allowed_extensions": [".bin", ".img", ".cfg"],
    "blocked_extensions": [".exe", ".sh", ".bat", ".ps1"]
  }
}
```

## Access Control

### Read-Only Mode

For maximum security, disable writes:

```json
{
  "security": {
    "read_enabled": true,
    "write_enabled": false
  }
}
```

### Client Filtering

```json
{
  "security": {
    "allowed_clients": [
      "192.168.1.0/24",
      "10.0.0.0/8"
    ],
    "blocked_clients": [
      "192.168.1.100"
    ]
  }
}
```

## Authentication and Authorization (Enterprise+)

### Enable Authentication

```json
{
  "security": {
    "authentication": {
      "enabled": true,
      "method": "ldap",
      "ldap_server": "ldap://ldap.example.com"
    }
  }
}
```

### Role-Based Access Control (RBAC)

```json
{
  "security": {
    "authorization": {
      "rbac_enabled": true,
      "roles": {
        "admin": {
          "permissions": ["*"]
        },
        "operator": {
          "permissions": ["server:read", "config:read"]
        }
      }
    }
  }
}
```

## Encryption

### TLS/SSL (Enterprise+)

```json
{
  "web": {
    "ssl_enabled": true,
    "ssl_cert": "/etc/simple-tftpd/ssl/server.crt",
    "ssl_key": "/etc/simple-tftpd/ssl/server.key",
    "tls_version": "1.3"
  }
}
```

### Certificate Management

- Use valid SSL certificates
- Rotate certificates regularly
- Use strong key sizes (2048+ bits)
- Monitor certificate expiration

## Logging and Monitoring

### Security Logging

```json
{
  "logging": {
    "level": "INFO",
    "log_file": "/var/log/simple-tftpd/simple-tftpd.log",
    "security_events": true,
    "audit_log": "/var/log/simple-tftpd/audit.log"
  }
}
```

### Monitor Security Events

- Failed authentication attempts
- Access violations
- Unauthorized access attempts
- Configuration changes

## Configuration Security

### Secure Configuration Files

```bash
# Set proper permissions
sudo chmod 600 /etc/simple-tftpd/simple-tftpd.conf
sudo chown root:root /etc/simple-tftpd/simple-tftpd.conf
```

### Secrets Management

- Don't hardcode passwords
- Use environment variables
- Use secret management systems
- Rotate secrets regularly

### Configuration Validation

```bash
# Validate configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Security audit
simple-tftpd security-audit --config /etc/simple-tftpd/simple-tftpd.conf
```

## System Security

### Run as Non-Root

```bash
# Create dedicated user
sudo useradd -r -s /bin/false -d /var/tftp tftp

# Run service as tftp user
# In systemd service file:
User=tftp
Group=tftp
```

### File System Permissions

```bash
# Restrict access to configuration
sudo chmod 600 /etc/simple-tftpd/simple-tftpd.conf
sudo chown root:root /etc/simple-tftpd/simple-tftpd.conf

# Restrict access to logs
sudo chmod 640 /var/log/simple-tftpd/simple-tftpd.log
sudo chown tftp:adm /var/log/simple-tftpd/simple-tftpd.log
```

### SELinux/AppArmor

**SELinux:**
```bash
# Create SELinux policy
sudo semanage fcontext -a -t tftp_exec_t "/usr/bin/simple-tftpd"
sudo restorecon -v /usr/bin/simple-tftpd
```

**AppArmor:**
```bash
# Create AppArmor profile
sudo aa-genprof simple-tftpd
```

## Update Management

### Regular Updates

```bash
# Check for updates
sudo apt update
apt list --upgradable | grep simple-tftpd

# Apply security updates
sudo apt upgrade simple-tftpd
```

### Security Advisories

- Monitor security advisories
- Subscribe to security mailing lists
- Review release notes
- Test updates in non-production

## Incident Response

### Security Incident Procedures

1. **Identify**: Detect security incident
2. **Contain**: Isolate affected systems
3. **Investigate**: Determine root cause
4. **Remediate**: Fix security issues
5. **Recover**: Restore normal operations
6. **Document**: Document incident and response

### Logging for Forensics

- Enable detailed logging
- Retain logs for investigation
- Centralize log collection
- Monitor for anomalies

## Compliance

### Security Standards

- Follow industry security standards
- Implement security controls
- Regular security audits
- Document security procedures

### Data Protection

- Encrypt sensitive data
- Implement access controls
- Regular backups
- Secure backup storage

## Best Practices Summary

1. **Network Security**: Use firewalls, restrict access
2. **File System**: Proper permissions, path protection
3. **Access Control**: Client filtering, read-only when possible
4. **Authentication**: Enable authentication (Enterprise+)
5. **Encryption**: Use TLS/SSL (Enterprise+)
6. **Logging**: Enable security logging
7. **Configuration**: Secure configuration files
8. **System Security**: Run as non-root, use SELinux/AppArmor
9. **Updates**: Regular security updates
10. **Monitoring**: Monitor security events

## Security Checklist

- [ ] Firewall configured
- [ ] Client filtering enabled
- [ ] File permissions set correctly
- [ ] Path traversal protection enabled
- [ ] File extension filtering configured
- [ ] Read-only mode enabled (if applicable)
- [ ] Authentication enabled (Enterprise+)
- [ ] TLS/SSL configured (Enterprise+)
- [ ] Security logging enabled
- [ ] Configuration files secured
- [ ] Service runs as non-root
- [ ] Regular security updates
- [ ] Security monitoring configured

## Next Steps

- **[Production Security](../../production/security.md)** - Production security guide
- **[Enterprise Security](../../enterprise/security.md)** - Enterprise security features
- **[Troubleshooting](../../troubleshooting/README.md)** - Security troubleshooting

---

**Last Updated**: December 2024
