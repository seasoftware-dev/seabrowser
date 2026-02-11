#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_DIR/output"
STAGING_DIR="$OUTPUT_DIR/linux"
VERSION="1.0.0"
ARCH="x86_64"

echo "========================================="
echo "Building AppImage for Tsunami"
echo "========================================="

# Check if binary exists
if [ ! -f "$STAGING_DIR/usr/bin/Tsunami" ]; then
    echo "Error: Binary not found. Run ./scripts/build.sh first."
    exit 1
fi

# Create AppDir structure
APPDIR="$OUTPUT_DIR/appimage/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/pixmaps"
mkdir -p "$APPDIR/usr/share/tsunami/data/icons"
mkdir -p "$APPDIR/usr/share/tsunami/data/pages"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/plugins"  # Qt plugins

# Copy binary
cp "$STAGING_DIR/usr/bin/Tsunami" "$APPDIR/usr/bin/"

# Copy data folder with all contents
echo "Copying data folder..."
cp -r "$STAGING_DIR/usr/share/tsunami/data/." "$APPDIR/usr/share/tsunami/data/"

# Copy icons to pixmaps
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.svg" "$APPDIR/usr/share/pixmaps/"

# Create desktop entry
cat > "$APPDIR/usr/share/applications/io.tsunami.Tsunami.desktop" << 'DESKTOP'
[Desktop Entry]
Name=Tsunami
Comment=A fast, private, and beautiful web browser
Exec=Tsunami %u
Icon=io.tsunami.Tsunami
Terminal=false
Type=Application
Categories=Network;WebBrowser;
MimeType=text/html;text/xml;application/xhtml+xml;x-scheme-handler/http;x-scheme-handler/https;
Keywords=web;browser;internet;privacy;
StartupNotify=true
StartupWMClass=tsunami
Actions=new-window;new-private-window;

[Desktop Action new-window]
Name=New Window
Exec=Tsunami --new-window

[Desktop Action new-private-window]
Name=New Private Window
Exec=Tsunami --private-window
DESKTOP

# Copy Qt plugins
if [ -d "$STAGING_DIR/usr/lib" ]; then
    cp -r "$STAGING_DIR/usr/lib"/* "$APPDIR/usr/lib/" 2>/dev/null || true
fi

# Copy Qt plugins from system Qt if available
if [ -d "/usr/lib/qt6/plugins/platforms" ]; then
    mkdir -p "$APPDIR/usr/plugins/platforms"
    cp -f "/usr/lib/qt6/plugins/platforms/"* "$APPDIR/usr/plugins/platforms/" 2>/dev/null || true
fi

# Copy Qt libraries
mkdir -p "$APPDIR/usr/lib"
for lib in $(ldd "$APPDIR/usr/bin/Tsunami" 2>/dev/null | grep "=> /" | awk '{print $3}'); do
    if [ -f "$lib" ]; then
        cp -f "$lib" "$APPDIR/usr/lib/" 2>/dev/null || true
    fi
done

# Create AppRun script
cat > "$APPDIR/AppRun" << 'APPRUN'
#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set up Qt environment
export QT_PLUGIN_PATH="$SCRIPT_DIR/usr/plugins:$SCRIPT_DIR/usr/lib/plugins"
export QT_QPA_PLATFORM=wayland,xcb

# Set library path
export LD_LIBRARY_PATH="$SCRIPT_DIR/usr/lib:$LD_LIBRARY_PATH"

# Set XDG data directories
export XDG_DATA_DIRS="$SCRIPT_DIR/usr/share:$XDG_DATA_DIRS"

# Run the browser
exec "$SCRIPT_DIR/usr/bin/Tsunami" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

# Copy icon
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.svg" "$APPDIR/"

# Download and use appimagetool if not present
if [ ! -f "/usr/bin/appimagetool" ]; then
    echo "Downloading appimagetool..."
    wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" \
          -O /tmp/appimagetool.AppImage
    chmod +x /tmp/appimagetool.AppImage
    APPIMAGETOOL="/tmp/appimagetool.AppImage"
else
    APPIMAGETOOL="/usr/bin/appimagetool"
fi

# Build AppImage
cd "$OUTPUT_DIR/appimage"

# Set environment for static build
export VERSION="$VERSION"

$APPIMAGETOOL AppDir "Tsunami-${VERSION}-${ARCH}.AppImage"

# Move to final location
mv "Tsunami-${VERSION}-${ARCH}.AppImage" "$OUTPUT_DIR/appimage/"

echo ""
echo "AppImage created: $OUTPUT_DIR/appimage/Tsunami-${VERSION}-${ARCH}.AppImage"
echo ""
echo "To run:"
echo "  chmod +x $OUTPUT_DIR/appimage/Tsunami-${VERSION}-${ARCH}.AppImage"
echo "  $OUTPUT_DIR/appimage/Tsunami-${VERSION}-${ARCH}.AppImage"
echo ""
echo "Features included:"
echo "  - Desktop entry integration"
echo "  - MIME type associations"
echo "  - All data files bundled"
