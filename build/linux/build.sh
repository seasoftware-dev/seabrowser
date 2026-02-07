#!/bin/bash
# Sea Browser - Linux Build Script
# Builds the browser from source

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-output"

echo "🌊 Sea Browser Build Script"
echo "==========================="

# Check dependencies
echo "Checking dependencies..."

check_pkg() {
    if ! pkg-config --exists "$1" 2>/dev/null; then
        echo "❌ Missing: $1"
        return 1
    fi
    echo "✓ Found: $1"
    return 0
}

MISSING=0
# Updated dependencies for GTK3 build
check_pkg gtk+-3.0 || MISSING=1
check_pkg sqlite3 || MISSING=1
# webkit2gtk-4.1 is preferred, fall back to 4.0 if needed
if pkg-config --exists webkit2gtk-4.1; then
    echo "✓ Found: webkit2gtk-4.1"
elif pkg-config --exists webkit2gtk-4.0; then
    echo "✓ Found: webkit2gtk-4.0"
else
    echo "❌ Missing: webkit2gtk-4.1 (or 4.0)"
    MISSING=1
fi

if [ $MISSING -eq 1 ]; then
    echo ""
    echo "Install missing dependencies:"
    echo "  Fedora: sudo dnf install gtk3-devel webkit2gtk4.1-devel sqlite-devel cmake gcc-c++"
    echo "  Arch:   sudo pacman -S gtk3 webkit2gtk-4.1 sqlite cmake base-devel"
    echo "  Ubuntu: sudo apt install libgtk-3-dev libwebkit2gtk-4.1-dev libsqlite3-dev cmake g++"
    exit 1
fi

echo ""
echo "Building..."

# Create and enter build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

echo ""
echo "✅ Build successful!"
echo "📦 Binary: $BUILD_DIR/seabrowser"
echo ""
echo "Run with: $BUILD_DIR/seabrowser"
echo "Install with: sudo make install"
