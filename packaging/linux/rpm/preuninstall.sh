#!/bin/bash
# Pre-uninstallation script for simple-tftpd (RPM)

set -e

PROJECT_NAME="simple-tftpd"

if [ "$1" -eq 0 ]; then
    if command -v systemctl >/dev/null 2>&1; then
        systemctl stop "${PROJECT_NAME}.service" 2>/dev/null || true
        systemctl disable "${PROJECT_NAME}.service" 2>/dev/null || true
    fi

    if [ -x /etc/init.d/${PROJECT_NAME} ]; then
        /etc/init.d/${PROJECT_NAME} stop 2>/dev/null || true
    fi

    if command -v chkconfig >/dev/null 2>&1; then
        chkconfig --del "${PROJECT_NAME}" 2>/dev/null || true
    fi
fi

exit 0
