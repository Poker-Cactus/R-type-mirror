#!/bin/bash
# Download Inter font from GitHub release

set -e

FONT_DIR="$(dirname "$0")/vendor/fonts"
FONT_FILE="$FONT_DIR/Inter-Regular.otf"

if [ -f "$FONT_FILE" ]; then
    echo "✅ Font already installed"
    exit 0
fi

echo "📦 Downloading Inter font..."
mkdir -p "$FONT_DIR"

TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

curl -sL "https://github.com/rsms/inter/releases/download/v4.0/Inter-4.0.zip" -o "$TEMP_DIR/Inter.zip"
unzip -j -q "$TEMP_DIR/Inter.zip" "extras/otf/Inter-Regular.otf" -d "$FONT_DIR"

echo "✅ Inter font installed!"
echo "   Location: $FONT_FILE"
