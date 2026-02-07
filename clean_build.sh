#!/bin/bash

# Sea Browser Clean Build Script
# ===================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_OUTPUT="$PROJECT_ROOT/build-output"
BUILD_DEB="$PROJECT_ROOT/build-deb"

echo "==================================="
echo "  Sea Browser Clean Build"
echo "==================================="
echo ""

# Clean previous builds
if [ -d "$BUILD_OUTPUT" ]; then
    echo "[INFO] Cleaning build-output..."
    rm -rf "$BUILD_OUTPUT"
fi

if [ -d "$BUILD_DEB" ]; then
    echo "[INFO] Cleaning build-deb..."
    rm -rf "$BUILD_DEB"
fi

# Run build script
echo "[INFO] Starting fresh build..."
"$PROJECT_ROOT/build/linux/build.sh"

echo ""
echo "[SUCCESS] Clean build completed."
echo ""
