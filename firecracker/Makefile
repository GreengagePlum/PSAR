BPATH = build/
API_SOCK = /tmp/firecracker.socket
CONFIG = vm_config.json

.PHONY: all clean confirm kernel rootfs test

all: kernel firecracker rootfs

kernel: vmlinux
vmlinux: | $(BPATH)
	cd $(BPATH) && ../scripts/kernel.sh

firecracker: | $(BPATH)
	cd $(BPATH) && ../scripts/firecracker.sh

rootfs: rootfs.ext4
rootfs.ext4:
	cd $(BPATH) && DOCKER_SCRIPT="../scripts/docker.sh" ../scripts/rootfs.sh

test:
	sudo rm -f ${API_SOCK} && sudo ./firecracker --api-sock ${API_SOCK} --config-file ${CONFIG}

$(BPATH):
	mkdir -p $@

clean: confirm
	rm -rf $(BPATH) vmlinux firecracker rootfs.ext4 firecracker.log

confirm:
	@REPLY="" ; \
	read -p "Are you sure? [y/n] > " -r ; \
	if [[ ! $$REPLY =~ ^[Yy]$$ ]]; then \
		echo "KO. Stopping" ; \
		exit 1 ; \
	else \
		echo "OK. Continuing" ; \
		exit 0 ; \
	fi
