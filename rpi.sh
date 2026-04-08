#!/bin/bash
echo "=== Установка зависимостей для Raspberry Pi 3 ==="

sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    git \
    libgles2-mesa-dev \
    libglfw3-dev \
    libssl-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

echo "Теперь запускай: ./setup.sh"      