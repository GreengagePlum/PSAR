ROOT_PSWD=""
# Empty or single DNS entry to be appended to /etc/resolv.conf
DNS="8.8.8.8"

### This script is used to populate a formatted rootfs file with base directories and files
### It is written to be run inside an Alpine Linux Docker container and is thus not executable from the host
###
### It does a few main things:
###     - It installs a basic init used for interactive sessions (if you don't override the default init via `init=` kernel boot arg)
###     - It installs basic utilities
###     - It installs a minimal init used for actual microVM management (for core PSAR project)
###     - It installs Python
###     - It configures the root password (for interactive sessions), a DNS nameserver, the basic init (for interactive sessions)
###     - It copies over the base files and directory structure from Alpine Linux inside the rootfs
###
### This script is to be run inside a Docker container and not on the host!

# Install necessary packages
apk add --no-cache openrc
apk add --no-cache util-linux
apk add --no-cache tini=0.19.0-r3

# Install Python + Pip for Flask web server
apk add --no-cache python3 py3-pip
python -m venv /venv
. /venv/bin/activate
python -m pip install --no-cache-dir --upgrade pip
pip install --no-cache-dir Flask gunicorn

# Set password for root (for testing purposes)
echo "root:${ROOT_PSWD}" | chpasswd

# Set DNS to access internet (for testing purposes, normally set via kernel boot args but just in case)
echo "nameserver ${DNS}" >>/etc/resolv.conf

# Set automount for /tmp directory (for openrc)
echo "tmpfs /tmp tmpfs rw,nosuid,noatime,nodev,size=1G,mode=1777 0 0" >>/etc/fstab

# Set up a login terminal on the serial console (ttyS0, for openrc):
ln -s agetty /etc/init.d/agetty.ttyS0
echo ttyS0 >/etc/securetty
rc-update add agetty.ttyS0 default

# Make sure special file systems are mounted on boot:
rc-update add devfs boot
rc-update add procfs boot
rc-update add sysfs boot
rc-update add localmount boot

# Then, copy the newly configured system to the rootfs image:
for d in bin etc lib root sbin usr venv; do tar c "/$d" | tar x -C /my-rootfs; done

# The above command may trigger the following message:
# tar: Removing leading "/" from member names
# However, this is just a warning, so you should be able to
# proceed with the setup process.

for dir in dev proc run sys var tmp; do mkdir /my-rootfs/${dir}; done

# All done, exit docker shell.
exit
