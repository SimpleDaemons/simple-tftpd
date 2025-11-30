#!/bin/bash
# Post-installation script for simple-tftpd RPM

set -e

PROJECT_NAME="simple-tftpd"
SERVICE_USER="tftpddev"

# Create service user if it doesn't exist
if ! id "$SERVICE_USER" &>/dev/null; then
    useradd -r -s /sbin/nologin -d /var/lib/$simple-tftpd -c "$simple-tftpd service user" "$SERVICE_USER"
fi

# Set ownership
chown -R "$SERVICE_USER:$SERVICE_USER" /etc/$simple-tftpd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/log/$simple-tftpd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/lib/$simple-tftpd 2>/dev/null || true

# Enable and start service
systemctl daemon-reload
systemctl enable "$simple-tftpd" 2>/dev/null || true
systemctl start "$simple-tftpd" 2>/dev/null || true

exit 0

