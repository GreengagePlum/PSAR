#!/usr/bin/env sh

SIZE_MB=200
MNT="/tmp/my-rootfs"

ROOTFS="rootfs.ext4"
dd if=/dev/zero of=${ROOTFS} bs=1M count=${SIZE_MB}
mkfs.ext4 -O none ${ROOTFS}
mkdir ${MNT}
sudo mount ${ROOTFS} ${MNT}
CONTAINER=$(docker run -d --rm -v ${MNT}:/my-rootfs alpine)
docker exec -i ${CONTAINER} /bin/sh <${DOCKER_SCRIPT}
docker stop ${CONTAINER}
sudo umount ${MNT}
ln -s ${ROOTFS} ..
