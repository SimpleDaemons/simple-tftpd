# Monitoring Setup Guide

This guide covers setting up monitoring for Simple TFTP Daemon.

## Overview

Monitoring Simple TFTP Daemon helps you:
- Track server health and availability
- Monitor performance metrics
- Identify issues before they become problems
- Plan capacity and scaling

## Built-in Monitoring

### Health Checks

```bash
# Check server health
simple-tftpd health

# HTTP health endpoint (Enterprise+)
curl http://localhost:8081/api/v1/health
```

### Metrics Endpoint

**Production Version:**
```bash
# Basic metrics
simple-tftpd metrics
```

**Enterprise/Datacenter Version:**
```bash
# Prometheus metrics
curl http://localhost:8081/api/v1/metrics/prometheus

# JSON metrics
curl http://localhost:8081/api/v1/metrics
```

## Prometheus Integration

### Configuration

Add to `prometheus.yml`:

```yaml
scrape_configs:
  - job_name: 'simple-tftpd'
    static_configs:
      - targets: ['localhost:8081']  # Enterprise/Datacenter
    metrics_path: '/api/v1/metrics/prometheus'
    scrape_interval: 15s
```

### Key Metrics

- `tftp_server_uptime_seconds` - Server uptime
- `tftp_connections_active` - Active connections
- `tftp_transfers_total` - Total transfers
- `tftp_bytes_transferred_total` - Total bytes transferred
- `tftp_errors_total` - Total errors
- `tftp_response_time_seconds` - Response time

## Grafana Dashboard

### Import Dashboard

1. Open Grafana
2. Go to Dashboards → Import
3. Import dashboard ID: `simple-tftpd-dashboard`
4. Configure data source (Prometheus)

### Dashboard Panels

- **Server Status**: Uptime, health status
- **Connections**: Active connections, connection rate
- **Transfers**: Transfer rate, bytes transferred
- **Performance**: Response time, throughput
- **Errors**: Error rate, error types

## Log Monitoring

### Log Aggregation

**ELK Stack (Elasticsearch, Logstash, Kibana):**

```yaml
# Logstash configuration
input {
  file {
    path => "/var/log/simple-tftpd/simple-tftpd.log"
    codec => json
  }
}

filter {
  # Parse log entries
}

output {
  elasticsearch {
    hosts => ["elasticsearch:9200"]
    index => "simple-tftpd-%{+YYYY.MM.dd}"
  }
}
```

**Loki (Grafana):**

```yaml
# Promtail configuration
scrape_configs:
  - job_name: simple-tftpd
    static_configs:
      - targets:
          - localhost
        labels:
          job: simple-tftpd
          __path__: /var/log/simple-tftpd/*.log
```

## Alerting

### Prometheus Alerts

Create `alerts.yml`:

```yaml
groups:
  - name: simple-tftpd
    rules:
      - alert: TFTPDown
        expr: up{job="simple-tftpd"} == 0
        for: 1m
        annotations:
          summary: "TFTP server is down"

      - alert: HighErrorRate
        expr: rate(tftp_errors_total[5m]) > 10
        for: 5m
        annotations:
          summary: "High error rate detected"

      - alert: HighConnectionCount
        expr: tftp_connections_active > 1000
        for: 5m
        annotations:
          summary: "High connection count"
```

### Alertmanager Configuration

```yaml
route:
  receiver: 'default-receiver'
  routes:
    - match:
        alertname: TFTPDown
      receiver: 'critical-alerts'

receivers:
  - name: 'default-receiver'
    email_configs:
      - to: 'admin@example.com'
  
  - name: 'critical-alerts'
    email_configs:
      - to: 'oncall@example.com'
    slack_configs:
      - api_url: 'https://hooks.slack.com/services/...'
```

## SNMP Monitoring (Enterprise+)

### SNMP Configuration

```json
{
  "snmp": {
    "enabled": true,
    "listen_address": "0.0.0.0",
    "listen_port": 161,
    "community": "public"
  }
}
```

### SNMP MIB

The Enterprise Version includes a custom MIB with:
- Server status
- Connection statistics
- Transfer metrics
- Error counts
- Performance data

### SNMP Tools

```bash
# Get server status
snmpget -v2c -c public localhost 1.3.6.1.4.1.12345.1.1.0

# Walk server metrics
snmpwalk -v2c -c public localhost 1.3.6.1.4.1.12345.1
```

## System Monitoring

### Resource Monitoring

**CPU Usage:**
```bash
top -p $(pgrep simple-tftpd)
```

**Memory Usage:**
```bash
ps aux | grep simple-tftpd
```

**Network Usage:**
```bash
iftop -i eth0 -f "port 69"
```

### System Metrics

Monitor system-level metrics:
- CPU usage
- Memory usage
- Disk I/O
- Network I/O
- File descriptors

## Application Performance Monitoring (APM)

### Distributed Tracing (Datacenter)

```json
{
  "tracing": {
    "enabled": true,
    "backend": "jaeger",
    "endpoint": "http://jaeger:14268/api/traces"
  }
}
```

### APM Integration

- **Jaeger**: Distributed tracing
- **Zipkin**: Request tracing
- **New Relic**: Application monitoring
- **Datadog**: APM and infrastructure monitoring

## Custom Monitoring

### Custom Metrics

**Production Version:**
```bash
# Parse logs for custom metrics
tail -f /var/log/simple-tftpd.log | grep "ERROR" | wc -l
```

**Enterprise/Datacenter Version:**
```bash
# Custom metrics endpoint
curl http://localhost:8081/api/v1/metrics/custom
```

### Scripts

Create monitoring scripts:

```bash
#!/bin/bash
# check_tftp.sh

STATUS=$(simple-tftpd health)
if [ $? -ne 0 ]; then
  echo "CRITICAL: TFTP server is down"
  exit 2
fi

CONNECTIONS=$(simple-tftpd metrics | grep connections | awk '{print $2}')
if [ $CONNECTIONS -gt 1000 ]; then
  echo "WARNING: High connection count: $CONNECTIONS"
  exit 1
fi

echo "OK: Server is healthy"
exit 0
```

## Best Practices

1. **Monitor Key Metrics**: Uptime, connections, errors, performance
2. **Set Appropriate Alerts**: Not too sensitive, not too delayed
3. **Retain Metrics**: Keep metrics for capacity planning
4. **Dashboard Design**: Create clear, actionable dashboards
5. **Log Aggregation**: Centralize logs for analysis
6. **Regular Reviews**: Review metrics and alerts regularly

## Next Steps

- **[Backup Procedures](backup-procedures.md)** - Backup and restore
- **[Maintenance Procedures](maintenance-procedures.md)** - Maintenance tasks
- **[Troubleshooting](../../troubleshooting/README.md)** - Common issues

---

**Last Updated**: December 2024
