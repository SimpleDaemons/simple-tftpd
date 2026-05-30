#!/bin/bash
# Post-installation script for simple-tftpd (RPM)

set -e

PROJECT_NAME="simple-tftpd"
CONFIG_DIR="/etc/${PROJECT_NAME}"
CONFIG_FILE="${CONFIG_DIR}/simple-tftpd.conf"
EXAMPLE_JSON="${CONFIG_DIR}/simple-tftpd.conf.example.json"
EXAMPLE_CONF="${CONFIG_DIR}/simple-tftpd.conf.example"
SERVICE_USER="simple-tftpd"

if ! getent passwd "${SERVICE_USER}" >/dev/null 2>&1; then
    useradd --system --no-create-home --shell /sbin/nologin \
        --home-dir "/var/lib/${PROJECT_NAME}" \
        -c "Simple TFTP Daemon" "${SERVICE_USER}"
fi

mkdir -p "${CONFIG_DIR}" "${CONFIG_DIR}/examples" "${CONFIG_DIR}/instances"
mkdir -p "/var/lib/${PROJECT_NAME}" "/var/log/${PROJECT_NAME}" "/var/tftp"

if [ ! -f "${CONFIG_FILE}" ]; then
    if [ -f "${EXAMPLE_JSON}" ]; then
        cp "${EXAMPLE_JSON}" "${CONFIG_FILE}"
    elif [ -f "${EXAMPLE_CONF}" ]; then
        cp "${EXAMPLE_CONF}" "${CONFIG_FILE}"
    fi
fi

chown -R root:"${SERVICE_USER}" "${CONFIG_DIR}"
chmod 750 "${CONFIG_DIR}"
chmod 640 "${CONFIG_FILE}" 2>/dev/null || true
chown -R "${SERVICE_USER}:${SERVICE_USER}" "/var/lib/${PROJECT_NAME}" "/var/log/${PROJECT_NAME}"
chown "${SERVICE_USER}:${SERVICE_USER}" "/var/tftp" 2>/dev/null || true

if [ -x /etc/init.d/${PROJECT_NAME} ] && command -v chkconfig >/dev/null 2>&1; then
    chkconfig --add "${PROJECT_NAME}" 2>/dev/null || true
fi

if command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload
    systemctl enable "${PROJECT_NAME}.service" 2>/dev/null || true
fi

exit 0
