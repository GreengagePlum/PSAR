#!/usr/bin/env sh

set -e
set -u

# Check if already backed up
if [ ! -f "$BACKUP_FILE_RUN" ]; then
    echo "[*] Backing up existing $RUN_RESOLV_CONF to $BACKUP_FILE_RUN"
    sudo cp "$RUN_RESOLV_CONF" "$BACKUP_FILE_RUN"
else
    echo "[*] Backup already exists at $BACKUP_FILE_RUN"
fi

echo "[+] Completed snapshot of systemd-resolved internal nameserver list."
