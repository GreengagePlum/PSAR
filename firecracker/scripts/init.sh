#!/bin/sh

echo "Mounting essential pseudo filesystems..."
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t tmpfs tmpfs /tmp
# devtmpfs is mounted automatically by the kernel, how nice!

echo "Starting zombie reaping and signal passing..."
exec /sbin/tini -- /launcher.sh $1
