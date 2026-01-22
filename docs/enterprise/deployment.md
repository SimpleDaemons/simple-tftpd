# Enterprise Version Deployment Guide

**Version:** Planned  
**License:** BSL 1.1

---

## Overview

This guide covers deploying the Enterprise Version of Simple TFTP Daemon in enterprise environments. The Enterprise Version includes all Production features plus web management, REST API, SNMP integration, high availability, and advanced security.

## Pre-Deployment Checklist

- [ ] System requirements met
- [ ] Network configuration planned
- [ ] High availability architecture designed
- [ ] Database configured (if using persistent storage)
- [ ] SSL/TLS certificates prepared
- [ ] Authentication system configured
- [ ] Monitoring and alerting setup planned
- [ ] Backup strategy defined
- [ ] Disaster recovery plan documented

## System Requirements

### Minimum Requirements
- **CPU:** 4 cores, 2.5 GHz
- **RAM:** 4 GB
- **Disk:** 20 GB free space
- **Network:** Gigabit Ethernet
- **OS:** Linux (kernel 5.4+), macOS 12.0+, Windows Server 2019+

### Recommended Requirements
- **CPU:** 8+ cores, 3.0+ GHz
- **RAM:** 16+ GB
- **Disk:** 100+ GB SSD
- **Network:** 10 Gigabit Ethernet
- **OS:** Linux (kernel 5.10+)

## Deployment Architecture

### Single Server Deployment

```
┌─────────────────────────────────┐
│   Simple TFTP Daemon Enterprise │
│   - TFTP Server                 │
│   - Web Management UI           │
│   - REST API                    │
│   - SNMP Agent                  │
└─────────────────────────────────┘
```

### High Availability Deployment

```
┌──────────────┐         ┌──────────────┐
│   Server 1   │◄────────►│   Server 2   │
│  (Primary)    │         │ (Secondary)  │
└──────────────┘         └──────────────┘
       │                        │
       └──────────┬─────────────┘
                  │
         ┌────────▼────────┐
         │  Load Balancer  │
         └─────────────────┘
```

## Installation

### Step 1: Install Enterprise Version

```bash
# Download Enterprise package
wget https://repos.simpledaemons.com/enterprise/simple-tftpd-enterprise-latest.tar.gz

# Extract
tar -xzf simple-tftpd-enterprise-latest.tar.gz
cd simple-tftpd-enterprise

# Install
sudo ./install.sh
```

### Step 2: Configure Database (Optional)

For persistent storage and clustering:

```bash
# PostgreSQL example
sudo -u postgres createdb simple_tftpd
sudo -u postgres createuser simple_tftpd

# Configure in config file
{
  "database": {
    "type": "postgresql",
    "host": "localhost",
    "port": 5432,
    "database": "simple_tftpd",
    "username": "simple_tftpd",
    "password": "secure_password"
  }
}
```

### Step 3: Configure SSL/TLS

```bash
# Generate certificates
sudo mkdir -p /etc/simple-tftpd/ssl
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout /etc/simple-tftpd/ssl/server.key \
  -out /etc/simple-tftpd/ssl/server.crt

# Configure in config file
{
  "web": {
    "ssl_enabled": true,
    "ssl_cert": "/etc/simple-tftpd/ssl/server.crt",
    "ssl_key": "/etc/simple-tftpd/ssl/server.key"
  }
}
```

## Configuration

### Basic Enterprise Configuration

```json
{
  "network": {
    "listen_address": "0.0.0.0",
    "listen_port": 69,
    "ipv6_enabled": true
  },
  "web": {
    "enabled": true,
    "listen_address": "0.0.0.0",
    "listen_port": 8080,
    "ssl_enabled": true
  },
  "api": {
    "enabled": true,
    "listen_address": "0.0.0.0",
    "listen_port": 8081,
    "authentication": "bearer"
  },
  "high_availability": {
    "enabled": true,
    "mode": "active-passive",
    "cluster_nodes": [
      "192.168.1.10",
      "192.168.1.11"
    ]
  },
  "security": {
    "authentication": {
      "enabled": true,
      "method": "ldap",
      "ldap_server": "ldap://ldap.example.com"
    },
    "authorization": {
      "rbac_enabled": true,
      "default_role": "viewer"
    }
  }
}
```

## High Availability Setup

### Active-Passive Configuration

**Primary Server:**
```json
{
  "high_availability": {
    "enabled": true,
    "mode": "active",
    "cluster_id": "tftp-cluster-1",
    "node_id": "node-1",
    "peer_nodes": ["192.168.1.11"]
  }
}
```

**Secondary Server:**
```json
{
  "high_availability": {
    "enabled": true,
    "mode": "passive",
    "cluster_id": "tftp-cluster-1",
    "node_id": "node-2",
    "peer_nodes": ["192.168.1.10"]
  }
}
```

### Load Balancer Configuration

**Nginx Example:**
```nginx
upstream tftp_backend {
    server 192.168.1.10:8080;
    server 192.168.1.11:8080 backup;
}

server {
    listen 80;
    server_name tftp.example.com;

    location / {
        proxy_pass http://tftp_backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

## Web Management Interface

### Accessing the Web UI

1. **Open browser**: Navigate to `https://your-server:8080`
2. **Login**: Use configured authentication credentials
3. **Dashboard**: View server status, metrics, and connections

### Features

- **Server Management**: Start, stop, restart server
- **Configuration**: Edit configuration through web interface
- **Monitoring**: Real-time metrics and graphs
- **Connection Management**: View and manage active connections
- **User Management**: Manage users and roles (RBAC)
- **Logs**: View and search logs

## REST API

### API Endpoints

**Base URL**: `https://your-server:8081/api/v1`

**Authentication**:
```bash
# Get token
curl -X POST https://your-server:8081/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "admin", "password": "password"}'

# Use token
curl -H "Authorization: Bearer <token>" \
  https://your-server:8081/api/v1/server/status
```

**Common Endpoints**:
- `GET /server/status` - Server status
- `GET /server/metrics` - Server metrics
- `GET /connections` - List connections
- `POST /server/reload` - Reload configuration
- `GET /config` - Get configuration
- `PUT /config` - Update configuration

## SNMP Integration

### SNMP Configuration

```json
{
  "snmp": {
    "enabled": true,
    "listen_address": "0.0.0.0",
    "listen_port": 161,
    "community": "public",
    "trap_enabled": true,
    "trap_destination": "192.168.1.100"
  }
}
```

### SNMP MIB

The Enterprise Version includes a custom MIB for monitoring:
- Server status
- Connection counts
- Transfer statistics
- Error rates
- Performance metrics

## Security Configuration

### Authentication

**LDAP Example:**
```json
{
  "security": {
    "authentication": {
      "enabled": true,
      "method": "ldap",
      "ldap_server": "ldap://ldap.example.com:389",
      "ldap_base_dn": "dc=example,dc=com",
      "ldap_bind_dn": "cn=admin,dc=example,dc=com"
    }
  }
}
```

### Authorization (RBAC)

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
          "permissions": ["server:read", "config:read", "server:reload"]
        },
        "viewer": {
          "permissions": ["server:read", "config:read"]
        }
      }
    }
  }
}
```

## Monitoring

### Metrics Collection

The Enterprise Version collects:
- Server uptime
- Active connections
- Total transfers
- Bytes transferred
- Error rates
- Response times
- Resource usage

### Integration with Monitoring Systems

**Prometheus:**
```yaml
scrape_configs:
  - job_name: 'simple-tftpd'
    metrics_path: '/api/v1/metrics/prometheus'
    static_configs:
      - targets: ['localhost:8081']
```

**Grafana Dashboard**: Import pre-built dashboard for visualization

## Backup and Recovery

### Backup Configuration

```json
{
  "backup": {
    "enabled": true,
    "schedule": "0 2 * * *",  // Daily at 2 AM
    "retention_days": 30,
    "location": "/var/backups/simple-tftpd"
  }
}
```

### Manual Backup

```bash
# Backup configuration
sudo cp /etc/simple-tftpd/simple-tftpd.conf /var/backups/

# Backup database (if using)
pg_dump simple_tftpd > /var/backups/simple_tftpd_$(date +%Y%m%d).sql
```

## Troubleshooting

### Common Issues

**Web UI Not Accessible:**
```bash
# Check service status
sudo systemctl status simple-tftpd-enterprise

# Check firewall
sudo firewall-cmd --list-all

# Check logs
sudo journalctl -u simple-tftpd-enterprise -n 50
```

**HA Failover Not Working:**
```bash
# Check cluster status
simple-tftpd-enterprise cluster-status

# Verify network connectivity
ping <peer-node-ip>

# Check logs
tail -f /var/log/simple-tftpd/ha.log
```

## Next Steps

- **[High Availability Guide](high-availability.md)** - Detailed HA setup
- **[Management Interface](management.md)** - Web UI and API
- **[Integrations](integrations.md)** - SNMP and other integrations
- **[Operations Guide](operations.md)** - Day-to-day operations

## Support

- **Documentation**: [Complete Documentation](../README.md)
- **Enterprise Support**: Contact enterprise@simpledaemons.com
- **Troubleshooting**: [Troubleshooting Guide](../shared/troubleshooting/README.md)

---

**Last Updated**: December 2024
