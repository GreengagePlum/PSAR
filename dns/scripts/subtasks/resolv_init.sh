#!/usr/bin/env sh

set -e
set -u

# Check if already backed up
if [ ! -f "$BACKUP_FILE_RESOLV" ]; then
    echo "[*] Backing up existing $RESOLV_CONF to $BACKUP_FILE_RESOLV"
    sudo cp "$RESOLV_CONF" "$BACKUP_FILE_RESOLV"
else
    echo "[*] Backup already exists at $BACKUP_FILE_RESOLV"
fi

# Remove symlink if exists
if [ -L "$RESOLV_CONF" ]; then
    echo "[*] Backing up existing $RESOLV_CONF symlink to $BACKUP_FILE_SYMLINK_RESOLV"
    sudo cp -d "$RESOLV_CONF" "$BACKUP_FILE_SYMLINK_RESOLV"
    echo "[*] Removing symlink at $RESOLV_CONF"
    sudo rm $RESOLV_CONF
fi

# Write custom resolv.conf
echo "[*] Writing custom $RESOLV_CONF"
# echo -e "# This file is managed by microDNS and has been altered to ensure correct DNS behavior.\n" >"$RESOLV_CONF"
echo "nameserver $LOCAL_DNS_IP" | sudo tee "$RESOLV_CONF" >/dev/null

# Optional: detect systemd-resolved
if systemctl is-active --quiet systemd-resolved; then
    echo "[*] systemd-resolved is active. You should forward unresolved queries to 127.0.0.53"
else
    echo "[*] systemd-resolved not active. Consider forwarding to upstreams from original resolv.conf"
    echo "You can inspect: $BACKUP_FILE_RESOLV"
fi

echo "[+] DNS stub installed. System now uses $LOCAL_DNS_IP:$LOCAL_DNS_PORT for DNS."
