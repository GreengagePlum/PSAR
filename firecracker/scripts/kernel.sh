#!/usr/bin/env sh

KERNEL_VERSION="6.1.102"
KERNEL_CONFIG_URL="https://raw.githubusercontent.com/firecracker-microvm/firecracker/refs/tags/v1.11.0/resources/guest_configs/microvm-kernel-ci-x86_64-6.1.config"
export CC="ccache gcc"
export KCFLAGS="-pipe"
NJOBS=20

KERNEL_FOLDER="linux-${KERNEL_VERSION}"
KERNEL_ARCHIVE="${KERNEL_FOLDER}.tar.xz"
KERNEL_CONFIG="kernel.config"

wget "https://cdn.kernel.org/pub/linux/kernel/v6.x/${KERNEL_ARCHIVE}"
tar -xJf ${KERNEL_ARCHIVE}
wget ${KERNEL_CONFIG_URL} -O ${KERNEL_CONFIG}
cp ${KERNEL_CONFIG} ${KERNEL_FOLDER}/.config
cd ${KERNEL_FOLDER}
echo "CONFIG_PCI=y" >>.config
make -n olddefconfig
make -n -j${NJOBS} vmlinux && ln -s ${KERNEL_FOLDER}/vmlinux ..
