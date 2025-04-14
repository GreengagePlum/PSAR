#!/usr/bin/env sh

set -e
set -u

# Check if already backed up
if [ ! -f "$BACKUP_FILE_NSS" ]; then
    echo "[*] Backing up existing $NSS_CONF to $BACKUP_FILE_NSS"
    sudo cp "$NSS_CONF" "$BACKUP_FILE_NSS"
else
    echo "[*] Backup already exists at $BACKUP_FILE_NSS"
fi

# Remove symlink if exists
if [ -L "$NSS_CONF" ]; then
    echo "[*] Backing up existing $NSS_CONF symlink to $BACKUP_FILE_SYMLINK_NSS"
    sudo cp -d "$NSS_CONF" "$BACKUP_FILE_SYMLINK_NSS"
    echo "[*] Removing symlink at $NSS_CONF"
    sudo rm $NSS_CONF
fi

# Build new nsswitch.conf
echo "[*] Writing custom $NSS_CONF"

TMP_NEW="$(mktemp)"
echo -e "# This file is managed by microDNS and has been altered to ensure correct DNS behavior.\n" >"$TMP_NEW"

while IFS= read -r line; do
    case "$line" in
    hosts:*)
        echo "hosts: files dns" >>"$TMP_NEW"
        ;;
    \#* | "")
        # Skip all comments and empty lines
        ;;
    *)
        echo "$line" >>"$TMP_NEW"
        ;;
    esac
done <"$BACKUP_FILE_NSS"

# Apply changes
sudo cp "$TMP_NEW" "$NSS_CONF"
rm "$TMP_NEW"
sudo chmod --reference="$BACKUP_FILE_NSS" "$NSS_CONF"
echo "[*] Replaced 'hosts:' line in $NSS_CONF"

echo "[+] Name Service Switch configured. System now uses $RESOLV_CONF exclusively for DNS."
