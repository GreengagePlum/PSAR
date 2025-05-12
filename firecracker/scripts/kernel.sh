#!/usr/bin/env sh

### This script fetches the given Linux kernel version and the given Firecracker specialized kernel config
### It then extracts the kernel and compiles this kernel by assuming default values for any unspecified config parameters
### Lastly it symlinks the kernel image to its parent directory assuming that's where Firecracker will expect to find it
###
### USE THE MAKEFILE to launch this script since relative paths are a bit fragile

KERNEL_VERSION="6.1.102"
# The base kernel config tailored for Firecracker to fetch
# Make sure the version fetched by the URL you give is the same as the Firecracker binary that was fetched
# and also was made for the kernel version you gave above
KERNEL_CONFIG_URL="https://raw.githubusercontent.com/firecracker-microvm/firecracker/refs/tags/v1.11.0/resources/guest_configs/microvm-kernel-ci-x86_64-6.1.config"
export CC="ccache gcc"
export KCFLAGS="-pipe"
# Number of jobs for make when compiling the kernel
NJOBS=20

# Don't touch these variables
KERNEL_FOLDER="linux-${KERNEL_VERSION}"
KERNEL_ARCHIVE="${KERNEL_FOLDER}.tar.xz"
KERNEL_CONFIG="kernel.config"

wget "https://cdn.kernel.org/pub/linux/kernel/v6.x/${KERNEL_ARCHIVE}"
tar -xJf ${KERNEL_ARCHIVE}
wget ${KERNEL_CONFIG_URL} -O ${KERNEL_CONFIG}
cp ${KERNEL_CONFIG} ${KERNEL_FOLDER}/.config
cd ${KERNEL_FOLDER}
echo "CONFIG_PCI=y" >>.config
make olddefconfig
make -j${NJOBS} vmlinux && cd - && ln -s $(basename $(pwd))/${KERNEL_FOLDER}/vmlinux ..
