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

# Setup networking

if ! ifconfig tap0 > /dev/null 2>&1; then
echo "Setting up networking"
tunctl || err_exit "Error creating tap0"
ip addr flush dev tap0 || err_exit "ip addr flush"
ip addr add 192.168.1.1/26 dev tap0 || err_exit "ip addr add"
ip link set dev tap0 up || err_exit "ip link set"
/bin/sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward' \
|| err_exit "ip_forward"

iptables -t nat -A POSTROUTING -s 192.168.1.0/26 -j MASQUERADE \
|| err_exit "iptables"
dnsmasq -F 192.168.1.2,192.168.1.2 -a 192.168.1.1 -z \
|| err_exit "dnsmasq"
fi

# Start the guest
qemu-system-x86_64 -enable-kvm -m 4096 -hda ubuntu_14_04.img -vnc :1 \
-netdev tap,id=e1000,ifname=tap0,script=no -device virtio-net,netdev=e1000 \
-netdev user,id=rtl8139 -device rtl8139,netdev=rtl8139