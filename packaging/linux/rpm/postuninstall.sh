#!/bin/bash
# Post-uninstallation script for simple-tftpd RPM

set -e

# Reload systemd
systemctl daemon-reload

exit 0

