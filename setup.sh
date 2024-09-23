#!/bin/bash
sudo pacman -S freeglut mesa libxrandr libxinerama libxcursor libxi
mkdir build
cd build
cmake ..
