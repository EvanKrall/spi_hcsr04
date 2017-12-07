#!/bin/bash

mkdir -p /sys/kernel/config/device-tree/overlays/spi
cat /lib/firmware/nextthingco/chip/sample-spi.dtbo > /sys/kernel/config/device-tree/overlays/spi/dtbo
