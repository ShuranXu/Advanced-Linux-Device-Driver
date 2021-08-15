#!/bin/sh

err_exit() {
    echo $1
    exit 1
}

warn() {
    echo $1
}

if [ `id -u` -ne 0 ]; then
    err_exit "This script must be run as root"
fi


INTERFACE="enp0s31f6"
USER="shuran"

# Setup networking

if ! ifconfig tap0 > /dev/null 2>&1; then
    echo "Setting up networking"
    ip link add br0 type bridge
    ip link set br0 up

    ip link set "$INTERFACE" master br0

    # Drop existing IP from INTERFACE
    ip addr flush dev "$INTERFACE"

    # Assign IP to br0
    ip addr add 192.168.2.10/24 brd + dev br0
    ip route add default via 192.168.2.1 dev br0

    # setup tap0 interface
    ip tuntap add dev tap0 mode tap user "$USER"
    ip link set dev tap0 up
    ip link set tap0 master br0
fi 

# Start the guest

echo "Setting up VM"

qemu-system-x86_64 \
-enable-kvm \
-m 4096 \
-hda ubuntu_14_04.img \
-vnc :1 \
-netdev tap,id=e1000,ifname=tap0,script=no \
-device virtio-net,netdev=e1000 \
-netdev user,id=rtl8139 \
-device rtl8139,netdev=rtl8139

