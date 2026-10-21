#!/bin/sh
# The container keeps its own /dev. The WSL host's /dev is bind-mounted at
# /dev/host-dev. These links make a tty that shows up after start usable
# without rebuilding again.
if [ ! -d /dev/host-dev ]; then
    exit 0
fi

ln -sfn /dev/host-dev/ttyACM0 /dev/ttyACM0
ln -sfn /dev/host-dev/ttyUSB0 /dev/ttyUSB0
mkdir -p /dev/bus
ln -sfn /dev/host-dev/bus/usb /dev/bus/usb
