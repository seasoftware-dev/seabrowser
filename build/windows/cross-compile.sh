#!/bin/bash
# Sea Browser - Cross-compile for Windows from Linux
# Requires: mingw-w64

set -e

echo "🌊 Sea Browser Windows Cross-Compile"
echo "====================================="

# Check for mingw
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Installing mingw-w64..."
    sudo dnf install -y mingw64-gcc-c++ mingw64-gtk3 mingw64-webkitgtk4 mingw64-sqlite
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-windows"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure for Windows
cmake "$PROJECT_ROOT" \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)

echo ""
echo "✅ Windows build complete!"
echo "📦 Binary: $BUILD_DIR/seabrowser.exe"
