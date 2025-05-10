#!/usr/bin/env sh

ARCH="$(uname -m)"
VERSION="v1.11.0"
release_url="https://github.com/firecracker-microvm/firecracker/releases"

curl -L ${release_url}/download/${VERSION}/firecracker-${VERSION}-${ARCH}.tgz |
    tar -xz

ln -s release-${VERSION}-${ARCH}/firecracker-${VERSION}-${ARCH} ../firecracker
