#!/bin/sh
# Post-install setup for simple-tftpd on FreeBSD (run after pkg add if needed)
#
#   sudo /usr/local/share/simple-tftpd/post-install.sh

set -e

PROJECT_NAME="simple-tftpd"
PREFIX="${PREFIX:-/usr/local}"
CONFIG_DIR="${PREFIX}/etc/${PROJECT_NAME}"
CONFIG_FILE="${CONFIG_DIR}/simple-tftpd.conf"
EXAMPLE_JSON="${CONFIG_DIR}/simple-tftpd.conf.example.json"
EXAMPLE_CONF="${CONFIG_DIR}/simple-tftpd.conf.example"
SERVICE_USER="simple-tftpd"
DAEMON="${PREFIX}/bin/simple-tftpd"

if ! id "${SERVICE_USER}" >/dev/null 2>&1; then
	pw useradd "${SERVICE_USER}" -c "Simple TFTP Daemon" \
		-d "/var/db/${PROJECT_NAME}" -s /usr/sbin/nologin
fi

mkdir -p "${CONFIG_DIR}" "${CONFIG_DIR}/examples" "${CONFIG_DIR}/instances"
mkdir -p "/var/db/${PROJECT_NAME}" "/var/log/${PROJECT_NAME}" "/var/tftp"

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
chown -R "${SERVICE_USER}:${SERVICE_USER}" "/var/db/${PROJECT_NAME}" "/var/log/${PROJECT_NAME}"
chown "${SERVICE_USER}:${SERVICE_USER}" "/var/tftp" 2>/dev/null || true
chmod 755 "/var/tftp" 2>/dev/null || true

if [ -x "${DAEMON}" ] && command -v setcap >/dev/null 2>&1; then
	setcap 'cap_net_bind_service=ep' "${DAEMON}" 2>/dev/null || true
fi

echo "To enable at boot:"
echo "  sysrc simple_tftpd_enable=YES"
echo "  service simple_tftpd start"
