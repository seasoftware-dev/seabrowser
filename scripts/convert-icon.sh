#!/bin/bash

# Convert SVG logo to ICO format for Windows
# Requires ImageMagick: sudo apt install imagemagick

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SVG_FILE="$PROJECT_DIR/data/logo.svg"
OUTPUT_DIR="$PROJECT_DIR/src/resources/windows"

echo "Converting SVG to ICO..."
echo "Input: $SVG_FILE"
echo "Output: $OUTPUT_DIR"

if ! command -v convert &> /dev/null; then
    echo "Error: ImageMagick not found. Install with: sudo apt install imagemagick"
    exit 1
fi

# Create multi-resolution ICO with common sizes
convert -background none \
    "$SVG_FILE" \
    -resize 256x256 \
    -extent 256x256 \
    "$OUTPUT_DIR/tsunami.ico"

convert -background none \
    "$SVG_FILE" \
    -resize 16x16 \
    -extent 16x16 \
    "$OUTPUT_DIR/tsunami_sm.ico"

echo "ICO files created:"
ls -la "$OUTPUT_DIR"/*.ico
