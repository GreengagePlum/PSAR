BPATH = build/

# Firecracker test - parameters
API_SOCK = /tmp/firecracker.socket
CONFIG = vm_test_config.json

# Firecracker test - network setup (don't touch!)
TAP_DEV = tap0
TAP_IP = 172.16.0.1
MASK_SHORT = /30


.PHONY: help all clean confirm kernel rootfs test network forwarding stop-forwarding

help:
	@echo ""
	@echo "make target"
	@echo ""
	@echo "      target :	help		this message"
	@echo " 		all		create the rootfs, the kernel and fetch Firecracker"
	@echo " 		test		run the microVM in interactive mode for testing"
	@echo " 		clean		remove ALL downloaded files and build artifacts (BE CAUTIOUS)"
	@echo " 		kernel		download the kernel archive and compile"
	@echo " 		rootfs		create and populate the rootfs"
	@echo " 		firecracker	download the firecracker binary"
	@echo " 		network		create tap interface to render the microVM reachable (for an interactive session)"
	@echo " 		forwarding	enable IPv4 forwarding"
	@echo " 		stop-forwarding	disable IPv4 forwarding"
	@echo ""
	@echo "=> All artifacts are under $(BPATH) along with a few symlinks here"
	@echo ""

all: kernel firecracker rootfs

kernel: vmlinux
vmlinux: | $(BPATH)
	cd $(BPATH) && ../scripts/kernel.sh

firecracker: | $(BPATH)
	cd $(BPATH) && ../scripts/firecracker.sh

rootfs: rootfs.ext4
rootfs.ext4: | $(BPATH)
	cd $(BPATH) && DOCKER_SCRIPT="../scripts/docker.sh" ../scripts/rootfs.sh

test: kernel firecracker rootfs network
	touch firecracker.log
	sudo rm -f ${API_SOCK} && sudo ./firecracker --api-sock ${API_SOCK} --config-file ${CONFIG}

network:
	sudo ip link del "${TAP_DEV}" 2> /dev/null || true && \
	sudo ip tuntap add dev "${TAP_DEV}" mode tap && \
	sudo ip addr add "${TAP_IP}${MASK_SHORT}" dev "${TAP_DEV}" && \
	sudo ip link set dev "${TAP_DEV}" up

forwarding:
	sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward" && \
	sudo iptables -P FORWARD ACCEPT

stop-forwarding:
	sudo sh -c "echo 0 > /proc/sys/net/ipv4/ip_forward" && \
	sudo iptables -P FORWARD DROP

$(BPATH):
	mkdir -p $@

clean: confirm
	rm -rf $(BPATH) vmlinux firecracker rootfs.ext4 firecracker.log
	sudo ip link del "${TAP_DEV}"

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
