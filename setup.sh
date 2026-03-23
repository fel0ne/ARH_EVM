#!/bin/bash

# 1. Инициализация сабмодулей, если их забыли склонировать
echo "--- Updating submodules ---"
git submodule update --init --recursive

# 2. Создание папки сборки
echo "--- Preparing build directory ---"
mkdir -p build
cd build

# 3. Конфигурация и компиляция
echo "--- Compiling project ---"
cmake ../backend
make -j$(nproc)

echo "--- Done! ---"
./server_app
