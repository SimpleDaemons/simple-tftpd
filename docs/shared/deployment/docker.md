# Docker Deployment Guide

This guide covers deploying Simple TFTP Daemon using Docker containers.

## Quick Start

### Production Version

```bash
# Pull image
docker pull simpledaemons/simple-tftpd:production

# Run container
docker run -d \
  --name simple-tftpd \
  --net host \
  -v /var/tftp:/var/tftp \
  -v /etc/simple-tftpd:/etc/simple-tftpd \
  simpledaemons/simple-tftpd:production
```

### Enterprise Version

```bash
docker pull simpledaemons/simple-tftpd:enterprise

docker run -d \
  --name simple-tftpd-enterprise \
  --net host \
  -v /var/tftp:/var/tftp \
  -v /etc/simple-tftpd:/etc/simple-tftpd \
  simpledaemons/simple-tftpd:enterprise
```

### Datacenter Version

```bash
docker pull simpledaemons/simple-tftpd:datacenter

docker run -d \
  --name simple-tftpd-datacenter \
  --net host \
  -v /var/tftp:/var/tftp \
  -v /etc/simple-tftpd:/etc/simple-tftpd \
  simpledaemons/simple-tftpd:datacenter
```

## Docker Compose

### Basic Setup

Create `docker-compose.yml`:

```yaml
version: '3.8'

services:
  simple-tftpd:
    image: simpledaemons/simple-tftpd:production
    container_name: simple-tftpd
    network_mode: host
    volumes:
      - ./tftp:/var/tftp
      - ./config:/etc/simple-tftpd
      - ./logs:/var/log/simple-tftpd
    restart: unless-stopped
    environment:
      - TFTP_ROOT=/var/tftp
      - TFTP_CONFIG=/etc/simple-tftpd/simple-tftpd.conf
```

Start with:
```bash
docker-compose up -d
```

### Production Setup

```yaml
version: '3.8'

services:
  simple-tftpd:
    image: simpledaemons/simple-tftpd:production
    container_name: simple-tftpd
    network_mode: host
    volumes:
      - /var/tftp:/var/tftp:ro
      - /etc/simple-tftpd:/etc/simple-tftpd:ro
      - /var/log/simple-tftpd:/var/log/simple-tftpd
    restart: always
    user: "1000:1000"
    environment:
      - TFTP_ROOT=/var/tftp
      - TFTP_CONFIG=/etc/simple-tftpd/simple-tftpd.conf
    healthcheck:
      test: ["CMD", "simple-tftpd", "health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s
```

### Enterprise Setup with Database

```yaml
version: '3.8'

services:
  postgres:
    image: postgres:15
    environment:
      POSTGRES_DB: simple_tftpd
      POSTGRES_USER: simple_tftpd
      POSTGRES_PASSWORD: ${DB_PASSWORD}
    volumes:
      - postgres_data:/var/lib/postgresql/data

  simple-tftpd:
    image: simpledaemons/simple-tftpd:enterprise
    depends_on:
      - postgres
    environment:
      - DATABASE_URL=postgresql://simple_tftpd:${DB_PASSWORD}@postgres:5432/simple_tftpd
    volumes:
      - ./tftp:/var/tftp
      - ./config:/etc/simple-tftpd
    ports:
      - "69:69/udp"
      - "8080:8080"  # Web UI
      - "8081:8081"  # REST API

volumes:
  postgres_data:
```

## Configuration

### Volume Mounts

**Required Volumes:**
- `/var/tftp` - TFTP root directory
- `/etc/simple-tftpd` - Configuration directory

**Optional Volumes:**
- `/var/log/simple-tftpd` - Log directory
- `/var/lib/simple-tftpd` - Data directory (Enterprise/Datacenter)

### Environment Variables

```bash
# Basic configuration
TFTP_ROOT=/var/tftp
TFTP_CONFIG=/etc/simple-tftpd/simple-tftpd.conf

# Network
TFTP_LISTEN_ADDRESS=0.0.0.0
TFTP_LISTEN_PORT=69

# Security
TFTP_READ_ENABLED=true
TFTP_WRITE_ENABLED=false

# Logging
TFTP_LOG_LEVEL=INFO
TFTP_LOG_FILE=/var/log/simple-tftpd/simple-tftpd.log
```

## Networking

### Host Network Mode

```yaml
network_mode: host
```

**Pros:**
- Best performance
- Direct access to host network
- No port mapping needed

**Cons:**
- Less isolation
- Port conflicts possible

### Bridge Network Mode

```yaml
ports:
  - "69:69/udp"
  - "8080:8080"  # Web UI (Enterprise)
```

**Pros:**
- Better isolation
- Port mapping control
- Works with Docker Compose

**Cons:**
- Slightly lower performance
- Requires port mapping

### Custom Network

```yaml
networks:
  tftp_network:
    driver: bridge

services:
  simple-tftpd:
    networks:
      - tftp_network
```

## Security

### Run as Non-Root

```yaml
user: "1000:1000"
```

### Read-Only Root Filesystem

```yaml
read_only: true
tmpfs:
  - /tmp
  - /var/run
```

### Security Options

```yaml
security_opt:
  - no-new-privileges:true
cap_drop:
  - ALL
cap_add:
  - NET_BIND_SERVICE
```

## Resource Limits

```yaml
deploy:
  resources:
    limits:
      cpus: '2'
      memory: 2G
    reservations:
      cpus: '1'
      memory: 1G
```

## Health Checks

```yaml
healthcheck:
  test: ["CMD", "simple-tftpd", "health"]
  interval: 30s
  timeout: 10s
  retries: 3
  start_period: 40s
```

## Logging

### Docker Logging Driver

```yaml
logging:
  driver: "json-file"
  options:
    max-size: "10m"
    max-file: "3"
```

### External Logging

```yaml
logging:
  driver: "syslog"
  options:
    syslog-address: "tcp://logserver:514"
```

## Multi-Container Deployment

### Load Balanced Setup

```yaml
version: '3.8'

services:
  tftp1:
    image: simpledaemons/simple-tftpd:production
    # ... configuration

  tftp2:
    image: simpledaemons/simple-tftpd:production
    # ... configuration

  nginx:
    image: nginx:alpine
    ports:
      - "69:69/udp"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
    depends_on:
      - tftp1
      - tftp2
```

## Kubernetes Deployment

### Basic Deployment

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: simple-tftpd
spec:
  replicas: 3
  selector:
    matchLabels:
      app: simple-tftpd
  template:
    metadata:
      labels:
        app: simple-tftpd
    spec:
      containers:
      - name: simple-tftpd
        image: simpledaemons/simple-tftpd:production
        ports:
        - containerPort: 69
          protocol: UDP
        volumeMounts:
        - name: tftp-data
          mountPath: /var/tftp
        - name: config
          mountPath: /etc/simple-tftpd
      volumes:
      - name: tftp-data
        persistentVolumeClaim:
          claimName: tftp-pvc
      - name: config
        configMap:
          name: tftp-config
```

## Troubleshooting

### View Logs

```bash
# Container logs
docker logs simple-tftpd

# Follow logs
docker logs -f simple-tftpd

# Last 100 lines
docker logs --tail 100 simple-tftpd
```

### Execute Commands

```bash
# Shell access
docker exec -it simple-tftpd /bin/sh

# Check status
docker exec simple-tftpd simple-tftpd status

# Test configuration
docker exec simple-tftpd simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
```

### Inspect Container

```bash
# Container info
docker inspect simple-tftpd

# Resource usage
docker stats simple-tftpd
```

## Best Practices

1. **Use specific image tags** (not `latest`)
2. **Mount configuration as read-only** when possible
3. **Use health checks** for automatic recovery
4. **Set resource limits** to prevent resource exhaustion
5. **Use secrets** for sensitive configuration
6. **Enable logging** for troubleshooting
7. **Use restart policies** for automatic recovery
8. **Run as non-root** user when possible

## Next Steps

- **[Production Deployment](../../production/deployment.md)** - Production deployment
- **[Monitoring](monitoring.md)** - Monitoring setup
- **[Troubleshooting](../../troubleshooting/README.md)** - Common issues

---

**Last Updated**: December 2024
