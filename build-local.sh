#!/usr/bin/bash

GLFW="C:/Libraries/glfw-3.3.9.bin.WIN64"
clang main.o window.o examples/shader.o \
    "$VULKAN_SDK/Lib/vulkan-1.lib" \
    "$GLFW/lib-vc2022/glfw3_mt.lib" \
    -o shader.exe