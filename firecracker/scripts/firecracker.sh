#!/usr/bin/env sh

### This script downloads a given version of the Firecracker binary to be used in launching microVMs
### It then symlinks the binary with the right name to its parent folder assuming that's where firecracker will be launched from
###
### USE THE MAKEFILE to launch this script since relative paths are a bit fragile

ARCH="$(uname -m)"
VERSION="v1.11.0"
release_url="https://github.com/firecracker-microvm/firecracker/releases"

curl -L ${release_url}/download/${VERSION}/firecracker-${VERSION}-${ARCH}.tgz |
    tar -xz

ln -s $(basename $(pwd))/release-${VERSION}-${ARCH}/firecracker-${VERSION}-${ARCH} ../firecracker
