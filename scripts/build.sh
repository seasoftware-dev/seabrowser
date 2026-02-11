#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_DIR="$PROJECT_DIR/output"

echo "========================================="
echo "Tsunami Browser Build Script"
echo "========================================="

# Clean previous build
echo "[1/4] Cleaning previous build..."
rm -rf "$BUILD_DIR"
rm -rf "$OUTPUT_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"
mkdir -p "$OUTPUT_DIR"

# Configure with CMake
echo "[2/4] Configuring with CMake..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      "$PROJECT_DIR"

# Build
echo "[3/4] Building Tsunami..."
make -j$(nproc)

# Install to staging directory
echo "[4/4] Installing to staging..."
DESTDIR="$OUTPUT_DIR/linux/usr" make install

echo ""
echo "Build complete!"
echo "Binary: $OUTPUT_DIR/linux/usr/bin/Tsunami"
echo ""
echo "To create packages, run:"
echo "  ./scripts/build-deb.sh"
echo "  ./scripts/build-rpm.sh"
echo "  ./scripts/build-appimage.sh"
echo "  ./scripts/build-flatpak.sh"
echo "  ./scripts/build-windows.ps1 (on Windows)"
