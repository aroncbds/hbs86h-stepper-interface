#!/bin/sh
# Fetch the pinned FreeRTOS kernel and apply the Timer0 tick-selection patch.
set -eu

root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tag=$(tr -d '[:space:]' < "$root/third_party/freertos.tag")
dest="$root/third_party/FreeRTOS-Kernel"
portmacro="$dest/portable/ThirdParty/GCC/ATmega/portmacro.h"

if [ ! -f "$dest/tasks.c" ]; then
    git clone --depth 1 --branch "$tag" \
        https://github.com/FreeRTOS/FreeRTOS-Kernel.git "$dest"
fi

if ! grep -q configAVR_USE_TIMER0_TICK "$portmacro"; then
    git -C "$dest" apply "$root/tools/freertos-timer0.patch"
fi

touch "$dest/.fetch-stamp"
