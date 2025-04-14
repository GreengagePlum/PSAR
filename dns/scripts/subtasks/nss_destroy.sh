#!/usr/bin/env sh

set -e
set -u

if [ ! -f "$BACKUP_FILE_NSS" ]; then
    echo "[!] Backup file $BACKUP_FILE_NSS not found. Nothing to restore."
    exit 1
fi

echo "[*] Restoring original $NSS_CONF"
sudo rm -f "$NSS_CONF"

if [ -L "$BACKUP_FILE_SYMLINK_NSS" ]; then
    sudo mv "$BACKUP_FILE_SYMLINK_NSS" "$NSS_CONF"
else
    sudo mv "$BACKUP_FILE_NSS" "$NSS_CONF"
fi

sudo rm -f "$BACKUP_FILE_NSS" "$BACKUP_FILE_SYMLINK_NSS"
echo "[+] Restored from backup."
