#!/bin/bash

# Sea Browser Build Wrapper
# ===================================
# This script is a convenience wrapper for the main build script.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build/linux/build.sh"

if [ -f "$BUILD_SCRIPT" ]; then
    "$BUILD_SCRIPT" "$@"
else
    echo "[ERROR] Build script not found at $BUILD_SCRIPT"
    exit 1
fi
