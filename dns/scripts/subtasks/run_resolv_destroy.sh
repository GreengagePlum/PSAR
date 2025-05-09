#!/usr/bin/env sh

set -e
set -u

if [ ! -f "$BACKUP_FILE_RUN" ]; then
    echo "[!] Backup file $BACKUP_FILE_RUN not found. Nothing to cleanup."
    exit 1
fi

echo "[*] Cleaning up backup file $BACKUP_FILE_RUN"
sudo rm "$BACKUP_FILE_RUN"
echo "[+] Cleaned up backup file."
