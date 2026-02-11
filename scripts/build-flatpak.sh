#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_DIR/output"
MANIFEST_FILE="$SCRIPT_DIR/flatpak/io.tsunami.Tsunami.json"

echo "========================================="
echo "Building Flatpak for Tsunami"
echo "========================================="

# Check for flatpak-builder
if ! command -v flatpak-builder &> /dev/null; then
    echo "Error: flatpak-builder not found. Install with:"
    echo "  sudo apt install flatpak-builder"
    exit 1
fi

# Create flatpak staging directory
FLATPAK_DIR="$OUTPUT_DIR/flatpak"
rm -rf "$FLATPAK_DIR"
mkdir -p "$FLATPAK_DIR"

# Create repo directory
REPO_DIR="$FLATPAK_DIR/repo"
mkdir -p "$REPO_DIR"

# Build the flatpak
cd "$FLATPAK_DIR"

# Initialize the runtime repo
flatpak-builder --repo="$REPO_DIR" \
                --force-clean \
                build-dir "$MANIFEST_FILE"

# Export the flatpak bundle
BUNDLE_NAME="io.tsunami.Tsunami.flatpak"
flatpak build-bundle "$REPO_DIR" "$BUNDLE_NAME" io.tsunami.Tsunami --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

echo ""
echo "Flatpak bundle created: $OUTPUT_DIR/flatpak/$BUNDLE_NAME"
echo ""
echo "To install:"
echo "  flatpak install $OUTPUT_DIR/flatpak/$BUNDLE_NAME"
echo ""
echo "To run after installation:"
echo "  flatpak run io.tsunami.Tsunami"
