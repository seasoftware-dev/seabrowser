#!/bin/bash

# Sea Browser Debian Package Builder
# ===================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-deb"

echo "==================================="
echo "  Sea Browser Debian Package Builder"
echo "==================================="
echo ""
echo "Project root: $PROJECT_ROOT"
echo "Build output: $BUILD_DIR"
echo ""

# Check for required dependencies
echo "[INFO] Checking dependencies..."

if ! command -v dpkg-deb &> /dev/null; then
    echo "[ERROR] dpkg-deb not found. Please install dpkg-dev."
    echo "        sudo apt install dpkg-dev"
    exit 1
fi

# Create build directory
if [ -d "$BUILD_DIR" ]; then
    echo "[INFO] Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR/DEBIAN"
mkdir -p "$BUILD_DIR/usr/bin"
mkdir -p "$BUILD_DIR/usr/share/applications"
mkdir -p "$BUILD_DIR/usr/share/icons/hicolor/scalable/apps"

# Build the project first
echo "[INFO] Building project..."
"$SCRIPT_DIR/build.sh"

# Copy files
echo "[INFO] Copying files..."
cp "$PROJECT_ROOT/build-output/seabrowser" "$BUILD_DIR/usr/bin/"
cp "$PROJECT_ROOT/data/io.seabrowser.SeaBrowser.desktop" "$BUILD_DIR/usr/share/applications/"
cp "$PROJECT_ROOT/data/io.seabrowser.SeaBrowser.svg" "$BUILD_DIR/usr/share/icons/hicolor/scalable/apps/"

# Create control file
echo "[INFO] Creating control file..."
cat > "$BUILD_DIR/DEBIAN/control" <<EOF
Package: seabrowser
Version: 1.0.0
Section: web
Priority: optional
Architecture: amd64
Depends: libgtk-3-0, libwebkit2gtk-4.1-0, libsqlite3-0, libjson-glib-1.0-0
Maintainer: Sea Browser Team <team@seabrowser.io>
Description: Privacy-focused web browser
 A lightweight, privacy-focused web browser built with WebKitGTK.
EOF

# Build package
echo "[INFO] Building .deb package..."
dpkg-deb --build "$BUILD_DIR" "$PROJECT_ROOT/seabrowser_1.0.0_amd64.deb"

echo ""
echo "==================================="
echo "  PACKAGE BUILT SUCCESSFULLY!"
echo "==================================="
echo ""
echo "Package (release): $PROJECT_ROOT/seabrowser_1.0.0_amd64.deb"
echo ""
