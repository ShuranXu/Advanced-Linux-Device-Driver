#!/bin/sh

# This is used to create device nodes when dynamic major number is requested. 
# Not needed when using static major numbers or sysfs interface (recommended) 
# where udev can automatically create a device file when this module is 
# loaded

module="mmaper"
device1="vmalloc_dev"
device2="kmalloc_dev"
mode="664"

if grep '^staff:' /etc/group > /dev/null; then
    group="staff"
else
    group="wheel"
fi

major=`cat /proc/devices | awk "\\$2==\"$module\" {print \\$1}"`

# Remove stale nodes and replace them, then give gid and perms

mknod /dev/${device1} c $major 1
chgrp $group /dev/${device1} 
chmod $mode  /dev/${device1}

mknod /dev/${device2} c $major 2
chgrp $group /dev/${device2} 
chmod $mode  /dev/${device2}