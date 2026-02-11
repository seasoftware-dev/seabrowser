#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_DIR/output"
STAGING_DIR="$OUTPUT_DIR/linux"
VERSION="1.0.0"

echo "========================================="
echo "Building DEB Package for Tsunami"
echo "========================================="

# Check if binary exists
if [ ! -f "$STAGING_DIR/usr/bin/Tsunami" ]; then
    echo "Error: Binary not found. Run ./scripts/build.sh first."
    exit 1
fi

# Create DEB staging directory
DEB_DIR="$OUTPUT_DIR/deb"
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/pixmaps"
mkdir -p "$DEB_DIR/usr/share/tsunami/data/icons"
mkdir -p "$DEB_DIR/usr/share/tsunami/data/pages"
mkdir -p "$DEB_DIR/usr/lib/tsunami/plugins"
mkdir -p "$DEB_DIR/DEBIAN"

# Copy binary
cp "$STAGING_DIR/usr/bin/Tsunami" "$DEB_DIR/usr/bin/"

# Copy data folder with all contents
echo "Copying data folder..."
cp -r "$STAGING_DIR/usr/share/tsunami/data/." "$DEB_DIR/usr/share/tsunami/data/"

# Copy icons to pixmaps for desktop integration
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.svg" "$DEB_DIR/usr/share/pixmaps/"

# Copy desktop file
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.desktop" "$DEB_DIR/usr/share/applications/"

# Copy Qt plugins if needed
if [ -d "$STAGING_DIR/usr/lib/tsunami/plugins" ]; then
    cp -r "$STAGING_DIR/usr/lib/tsunami/plugins"/* "$DEB_DIR/usr/lib/tsunami/plugins/" 2>/dev/null || true
fi

# Copy Qt libraries
mkdir -p "$DEB_DIR/usr/lib"
for lib in $(ldd "$DEB_DIR/usr/bin/Tsunami" 2>/dev/null | grep "=> /" | awk '{print $3}'); do
    if [ -f "$lib" ]; then
        cp -f "$lib" "$DEB_DIR/usr/lib/" 2>/dev/null || true
    fi
done

# Create control file
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: tsunami
Version: ${VERSION}-1
Section: net
Priority: optional
Architecture: amd64
Depends: libsqlite3-0, libqt6core6t64 (>= 6.4), libqt6webenginecore6t64 (>= 6.4), libqt6widgets6t64 (>= 6.4), libqt6gui6t64 (>= 6.4), libqt6opengl6t64 (>= 6.4), libx11-6, libxcb-glx0
Maintainer: Tsunami Developers <dev@tsunami.io>
Description: A fast, private, and beautiful web browser
 Tsunami is a privacy-focused web browser built with Qt6 WebEngine.
 It features a clean interface, dark/light themes, and robust
 privacy protections.
Homepage: https://tsunami.io
EOF

# Create postinst script
cat > "$DEB_DIR/DEBIAN/postinst" << 'POSTINST'
#!/bin/bash
set -e

# Update desktop database
if [ -f /usr/share/applications/io.tsunami.Tsunami.desktop ]; then
    update-desktop-database -q 2>/dev/null || true
fi

# Register MIME types
if [ -f /usr/share/applications/io.tsunami.Tsunami.desktop ]; then
    xdg-mime default io.tsunami.Tsunami.desktop x-scheme-handler/http 2>/dev/null || true
    xdg-mime default io.tsunami.Tsunami.desktop x-scheme-handler/https 2>/dev/null || true
    xdg-mime default io.tsunami.Tsunami.desktop text/html 2>/dev/null || true
    xdg-mime default io.tsunami.Tsunami.desktop application/xhtml+xml 2>/dev/null || true
fi

# Update icon cache
if [ -f /usr/share/pixmaps/io.tsunami.Tsunami.svg ]; then
    gtk-update-icon-cache -f -t /usr/share/pixmaps 2>/dev/null || true
fi

# Create symlink for icon
mkdir -p /usr/local/share/pixmaps 2>/dev/null || true
ln -sf /usr/share/pixmaps/io.tsunami.Tsunami.svg /usr/local/share/pixmaps/tsunami.svg 2>/dev/null || true

exit 0
POSTINST
chmod +x "$DEB_DIR/DEBIAN/postinst"

# Create prerm script
cat > "$DEB_DIR/DEBIAN/prerm" << 'PRERM'
#!/bin/bash
set -e

# Remove symlink
rm -f /usr/local/share/pixmaps/tsunami.svg 2>/dev/null || true

exit 0
PRERM
chmod +x "$DEB_DIR/DEBIAN/prerm"

# Create conffiles
cat > "$DEB_DIR/DEBIAN/conffiles" << 'CONFFILES'
/usr/share/tsunami/data/settings.json
CONFFILES

# Copy copyright
cp "$PROJECT_DIR/LICENSE" "$DEB_DIR/DEBIAN/copyright" 2>/dev/null || true

# Create DEB package
cd "$OUTPUT_DIR/deb"
PACKAGE_NAME="tsunami_${VERSION}-1_amd64.deb"
dpkg-deb --build "$DEB_DIR" "$PACKAGE_NAME"

echo ""
echo "DEB package created: $OUTPUT_DIR/deb/$PACKAGE_NAME"
echo ""
echo "To install:"
echo "  sudo dpkg -i $OUTPUT_DIR/deb/$PACKAGE_NAME"
echo ""
echo "Or install with gdebi for automatic dependency resolution:"
echo "  sudo apt install gdebi"
echo "  sudo gdebi $OUTPUT_DIR/deb/$PACKAGE_NAME"
