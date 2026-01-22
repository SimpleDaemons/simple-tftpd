# Enterprise Version Installation Guide

**Version:** Planned
**License:** BSL 1.1

---

## Overview

This guide covers installing the Enterprise Version of Simple TFTP Daemon.

## Prerequisites

- Linux (kernel 4.0+), macOS 11+, or Windows Server 2019+
- 2+ GB RAM
- 5+ GB disk space
- Network connectivity

## Installation Methods

### Package Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install simple-tftpd-enterprise
```

**CentOS/RHEL:**
```bash
sudo yum install simple-tftpd-enterprise
```

### Docker Installation

```bash
docker pull simpledaemons/simple-tftpd:enterprise
docker run -d --name simple-tftpd-enterprise --net host simpledaemons/simple-tftpd:enterprise
```

### Kubernetes Installation

See [Kubernetes Deployment](kubernetes.md) for Kubernetes installation.

---

## Post-Installation

1. Configure Enterprise features (see [Configuration Guide](configuration.md))
2. Set up Web Management Interface (see [Management Guide](management.md))
3. Configure High Availability (see [HA Guide](high-availability.md))

---

**Last Updated:** December 2024
**Status:** Planned
