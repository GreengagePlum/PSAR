# Scripts

These are scripts used for several different reasons related all to Firecracker or the microVMs.

Some of these reasons are:

* Creating the rootfs (`rootfs.sh`)
* Compiling the kernel (`kernel.sh`)
* Fetching the Firecracker binary (`firecracker.sh`)
* Minimal init used inside the microVMs (`init.sh`)
* Launcher script to create a single entry point for the minimal init to launch various programs with the right arguments that manage the microVMs (`launcher.sh`)

These scripts for the most part are made to be used ONLY via the `Makefile` inside the parent `firecracker/` directory. But if you wanna tinker, tinker away!
