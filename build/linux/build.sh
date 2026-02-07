#!/bin/bash

# Sea Browser Build Script (Linux)
# ===================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-output"

echo "==================================="
echo "  Sea Browser Build Script (Linux)"
echo "==================================="
echo ""
echo "Project root: $PROJECT_ROOT"
echo "Build output: $BUILD_DIR"
echo ""

# Check for required dependencies
echo "[INFO] Checking dependencies..."

check_pkg() {
    if ! pkg-config --exists "$1" 2>/dev/null; then
        echo "[ERROR] Missing dependency: $1"
        echo "        Install it using your package manager."
        return 1
    fi
    echo "  ✓ $1"
}

MISSING=0
check_pkg "gtk+-3.0" || MISSING=1
check_pkg "webkit2gtk-4.1" || MISSING=1
check_pkg "sqlite3" || MISSING=1
check_pkg "json-glib-1.0" || MISSING=1
check_pkg "x11" || MISSING=1

if [ $MISSING -eq 1 ]; then
    echo ""
    echo "[ERROR] Some dependencies are missing. Please install them and try again."
    echo ""
    echo "Debian/Ubuntu: sudo apt install libgtk-3-dev libwebkit2gtk-4.1-dev libsqlite3-dev libjson-glib-dev libx11-dev"
    echo "Fedora:        sudo dnf install gtk3-devel webkit2gtk4.1-devel sqlite-devel json-glib-devel libX11-devel"
    echo "Arch Linux:    sudo pacman -S gtk3 webkit2gtk-4.1 sqlite json-glib libx11"
    exit 1
fi

echo ""
echo "[INFO] All dependencies found."
echo ""

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "[INFO] Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
echo "[INFO] Configuring CMake..."
cmake "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "[ERROR] CMake configuration failed."
    exit 1
fi

# Build
echo ""
echo "[INFO] Building Sea Browser..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed."
    exit 1
fi

echo ""
echo "==================================="
echo "  BUILD SUCCESSFUL!"
echo "==================================="
echo ""
echo "Executable: $BUILD_DIR/seabrowser"
echo ""
echo "To run: $BUILD_DIR/seabrowser"
echo ""
