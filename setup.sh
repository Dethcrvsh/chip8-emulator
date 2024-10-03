#!/bin/bash
sudo pacman -S glfw-x11 mesa libxrandr libxinerama libxcursor libxi
export PKG_CONFIG_PATH=/usr/lib/pkgconfig:$PKG_CONFIG_PATH
mkdir build
cd build
cmake ..
