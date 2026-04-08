#!/bin/bash
set -e

echo "=============================="
echo "  ARH_EVM BUILD SCRIPT"
echo "=============================="

ROOT_DIR=$(pwd)

echo "[1/4] Updating submodules..."
git submodule update --init --recursive

echo "[2/4] Building backend..."
mkdir -p build/backend
cd build/backend
cmake ../../backend
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
echo "Backend built successfully."

echo "[3/4] Building frontend..."
mkdir -p ../frontend
cd ../frontend
cmake ../../frontend
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
echo "Frontend built successfully."

echo "[4/4] Build complete!"
echo ""
echo "Run backend:"
echo "  ./build/backend/server_app"
echo ""
echo "Run frontend:"
echo "  ./build/frontend/frontend"

cd "$ROOT_DIR"