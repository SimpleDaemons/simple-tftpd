#!/bin/bash
# Pre-uninstallation script for simple-tftpd RPM

set -e

PROJECT_NAME="simple-tftpd"

# Stop service before removal
if [ "$1" -eq 0 ]; then
    systemctl stop "$simple-tftpd" 2>/dev/null || true
    systemctl disable "$simple-tftpd" 2>/dev/null || true
fi

exit 0

