#!/bin/bash
# Sea Browser - DEB Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
VERSION="1.0.0"
DEB_DIR="$PROJECT_ROOT/build-deb/seabrowser_${VERSION}"

echo "🌊 Sea Browser DEB Build"
echo "========================"

# Build first
"$SCRIPT_DIR/build.sh"

# Create DEB structure
rm -rf "$PROJECT_ROOT/build-deb"
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/scalable/apps"

# Copy files
cp "$PROJECT_ROOT/build-output/seabrowser" "$DEB_DIR/usr/bin/"
cp "$PROJECT_ROOT/data/io.seabrowser.SeaBrowser.desktop" "$DEB_DIR/usr/share/applications/"
cp "$PROJECT_ROOT/data/io.seabrowser.SeaBrowser.svg" "$DEB_DIR/usr/share/icons/hicolor/scalable/apps/"

# Create control file
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: seabrowser
Version: ${VERSION}
Section: web
Priority: optional
Architecture: amd64
# Updated dependencies for GTK3
Depends: libgtk-3-0, libwebkit2gtk-4.1-0, libsqlite3-0
Maintainer: Sea Browser <seabrowser@example.com>
Description: Privacy-focused web browser
 Sea Browser is a privacy-focused web browser built with WebKitGTK.
EOF

# Build DEB
echo "Building DEB..."
dpkg-deb --build "$DEB_DIR"

echo ""
echo "✅ DEB built!"
echo "📦 Package: $PROJECT_ROOT/build-deb/seabrowser_${VERSION}.deb"
