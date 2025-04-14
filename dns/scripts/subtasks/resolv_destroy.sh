#!/usr/bin/env sh

set -e
set -u

if [ ! -f "$BACKUP_FILE_RESOLV" ]; then
    echo "[!] Backup file $BACKUP_FILE_RESOLV not found. Nothing to restore."
    exit 1
fi

echo "[*] Restoring original $RESOLV_CONF"
sudo rm -f "$RESOLV_CONF"

if [ -L "$BACKUP_FILE_SYMLINK_RESOLV" ]; then
    sudo mv "$BACKUP_FILE_SYMLINK_RESOLV" "$RESOLV_CONF"
else
    sudo mv "$BACKUP_FILE_RESOLV" "$RESOLV_CONF"
fi

sudo rm -f "$BACKUP_FILE_RESOLV" "$BACKUP_FILE_SYMLINK_RESOLV"
echo "[+] Restored from backup."

if systemctl is-active --quiet systemd-resolved; then
    echo "[*] systemd-resolved is active. Restarting..."
    sudo systemctl restart systemd-resolved.service
fi
