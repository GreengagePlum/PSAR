#!/usr/bin/env sh

### This script is used to create and populate the rootfs to be used inside the Firecracker microVMs
### It uses Alpine Linux Docker image as a base to create the base folder structure and files
### You thus need:
###     - Docker
###     - mkfs.ext4 utility
###     - sudo privileges
### At the end the rootfs file is symlinked to the parent directory of where you execute this script thus assuming that's where Firecracker will be launched from
###
### USE THE MAKEFILE to launch this script since relative paths are a bit fragile

# Size in MB of the rootfs to create
SIZE_MB=200
# Where to mount the rootfs temporarily while populating it
MNT="/tmp/my-rootfs"
# The Docker image tag to use for the Alpine Linux container
ALPINE_TAG="3.21.3"
# Name of the kernel init script inside this script's folder (must be at the top level)
INIT="init.sh"

# Don't touch these variables
INIT_FILE="../scripts/${INIT}"
ROOTFS="rootfs.ext4"

# Create file for the rootfs, format it and populate it with base files
dd if=/dev/zero of=${ROOTFS} bs=1M count=${SIZE_MB}
mkfs.ext4 -O none ${ROOTFS}
mkdir ${MNT}
sudo mount ${ROOTFS} ${MNT} || exit 1
CONTAINER=$(docker run -td --rm -v ${MNT}:/my-rootfs alpine:${ALPINE_TAG})
docker exec -i ${CONTAINER} /bin/sh <${DOCKER_SCRIPT}
docker stop ${CONTAINER}

# Copy extra PSAR related files (custom init, scripts, manager programs + functions)
sudo cp ${INIT_FILE} ${MNT}/
sudo chown root ${MNT}/${INIT}
sudo chmod u+x ${MNT}/${INIT}
sudo cp ../scripts/launcher.sh ${MNT}/
sudo chown root ${MNT}/launcher.sh
sudo chmod u+x ${MNT}/launcher.sh
sudo cp ../programs/app.py ${MNT}/
sudo chown root ${MNT}/app.py
FUNCS_FILES=$(find ../functions/ -maxdepth 1 -type f -executable)
FUNCS=$(for i in ${FUNCS_FILES}; do echo "${MNT}/$(basename $i)"; done)
sudo cp ${FUNCS_FILES} ${MNT}/
sudo chown root ${FUNCS}

# Unmount the temporary rootfs mount and symlink the rootfs file to parent directory
sudo umount ${MNT}
ln -s $(basename $(pwd))/${ROOTFS} ..
