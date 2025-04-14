#!/usr/bin/env sh

set -u
set -a

# Constants
CONFIG_FILE="../microDNS.conf"

# Load config
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Config file $CONFIG_FILE not found!"
    exit 1
fi
source "$CONFIG_FILE"

print_usage() {
    echo "Usage: $0 <command>"
    echo
    echo "Commands:"
    echo "  setup             Run both setup-resolv and setup-nss"
    echo "  teardown          Run both teardown-resolv and teardown-nss"
    echo
    echo "  setup-resolv      Replace $RESOLV_CONF with microDNS stub"
    echo "  teardown-resolv   Restore the original $RESOLV_CONF"
    echo
    echo "  setup-nss         Modify $NSS_CONF to route DNS through microDNS stub"
    echo "  teardown-nss      Restore the original $NSS_CONF"
    echo
}

# Ensure the script is called with exactly one argument
if [ "$#" -ne 1 ]; then
    print_usage
    exit 1
fi

setup_all() {
    ./subtasks/resolv_init.sh
    ./subtasks/nss_init.sh
}

teardown_all() {
    ./subtasks/nss_destroy.sh
    ./subtasks/resolv_destroy.sh
}

setup_resolv() {
    ./subtasks/resolv_init.sh
}

teardown_resolv() {
    ./subtasks/resolv_destroy.sh
}

setup_nss() {
    ./subtasks/nss_init.sh
}

teardown_nss() {
    ./subtasks/nss_destroy.sh
}

case "$1" in
setup)
    setup_all
    ;;
teardown)
    teardown_all
    ;;
setup-resolv)
    setup_resolv
    ;;
teardown-resolv)
    teardown_resolv
    ;;
setup-nss)
    setup_nss
    ;;
teardown-nss)
    teardown_nss
    ;;
*)
    print_usage
    exit 1
    ;;
esac
