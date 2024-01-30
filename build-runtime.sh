#!/usr/bin/bash

GLFW="C:/Libraries/glfw-3.3.9.bin.WIN64"
clang++ -v -std=c++17 -c \
    runtime/*.cpp \
    -I"$VULKAN_SDK/Include/" \
    -I"$GLFW/include/"