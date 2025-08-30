# Deployment Guide

This directory contains everything needed to deploy the Simple TFTP Daemon in production environments.

## Quick Start

```bash
# Automated installation
sudo ./deployment/scripts/install.sh

# Manual installation
make install
sudo cp deployment/systemd/simple-tftpd.service /etc/systemd/system/
sudo systemctl enable simple-tftpd
sudo systemctl start simple-tftpd
```

## Directory Structure

- `systemd/` - systemd service files
- `init.d/` - Traditional init.d scripts  
- `configs/` - Configuration files
- `examples/` - Deployment examples
- `scripts/` - Installation scripts
