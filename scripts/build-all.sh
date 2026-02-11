#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "========================================="
echo "Tsunami Browser - Complete Build"
echo "========================================="
echo ""

# Check dependencies
echo "Checking build dependencies..."
command -v cmake >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "make not found"; exit 1; }

# Run main build
echo ""
echo "[1/5] Building binary..."
"$SCRIPT_DIR/build.sh"

# Build packages
echo ""
echo "[2/5] Building DEB package..."
"$SCRIPT_DIR/build-deb.sh"

echo ""
echo "[3/5] Building RPM package..."
"$SCRIPT_DIR/build-rpm.sh"

echo ""
echo "[4/5] Building AppImage..."
"$SCRIPT_DIR/build-appimage.sh"

echo ""
echo "[5/5] Building Flatpak..."
"$SCRIPT_DIR/build-flatpak.sh"

echo ""
echo "========================================="
echo "All builds complete!"
echo "========================================="
echo ""
echo "Output directory: $PROJECT_DIR/output/"
echo ""
echo "Contents:"
echo "  linux/"
echo "    bin/               - Compiled binary with data files"
echo "    deb/               - DEB package (Debian/Ubuntu)"
echo "    rpm/               - RPM package (Fedora/openSUSE)"
echo "    flatpak/           - Flatpak bundle"
echo "    appimage/          - AppImage (portable)"
echo ""
echo "Windows build:"
echo "  Run: pwsh ./scripts/build-windows.ps1"
echo ""
echo "To install on Linux:"
echo "  DEB:    sudo dpkg -i output/deb/tsunami_1.0.0-1_amd64.deb"
echo "  RPM:    sudo dnf install output/rpm/RPMS/x86_64/tsunami-1.0.0-1.x86_64.rpm"
echo "  Flatpak: flatpak install output/flatpak/io.tsunami.Tsunami.flatpak"
echo "  AppImage: chmod +x output/appimage/Tsunami-1.0.0-x86_64.AppImage && ./output/appimage/Tsunami-1.0.0-x86_64.AppImage"
echo ""
echo "Package features:"
echo "  - Desktop entry integration"
echo "  - MIME type associations (HTTP/HTTPS/HTML)"
echo "  - Start menu/dash integration"
echo "  - All data files bundled (icons, pages, stylesheets)"
echo "  - Proper uninstall support"
