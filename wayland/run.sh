#!/bin/bash

# Log Wayland messages
export WAYLAND_DEBUG=1

# Run lvgl app
# ./lvgl

# Run lvgl app with strace
# ./strace \
#    -s 1024 \
#    ./lvgl

# Debug lvgl app
gdb \
    -ex="r" \
    -ex="bt" \
    -ex="frame" \
    --args ./lvgl
