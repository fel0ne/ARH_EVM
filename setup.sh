#!/bin/bash

set -e  # если ошибка — сразу стоп

echo "=============================="
echo "  ARH_EVM BUILD SCRIPT"
echo "=============================="

ROOT_DIR=$(pwd)

# ----------------------------
# 1. Submodules
# ----------------------------
echo "[1/4] Updating submodules..."
git submodule update --init --recursive

# ----------------------------
# 2. BACKEND BUILD
# ----------------------------
echo "[2/4] Building backend..."

mkdir -p build/backend
cd build/backend

cmake ../../backend
make -j$(sysctl -n hw.ncpu)

echo "Backend built successfully."

# ----------------------------
# 3. FRONTEND BUILD
# ----------------------------
echo "[3/4] Building frontend..."

mkdir -p ../frontend
cd ../frontend

cmake ../../frontend
make -j$(sysctl -n hw.ncpu)

echo "Frontend built successfully."

# ----------------------------
# 4. DONE
# ----------------------------
echo "[4/4] Build complete!"

echo ""
echo "Run backend:"
echo "  ./build/backend/server_app"

echo ""
echo "Run frontend:"
echo "  ./build/frontend/frontend"

cd "$ROOT_DIR"