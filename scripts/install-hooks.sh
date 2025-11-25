#!/bin/bash

HOOKS_DIR="$(git rev-parse --git-dir)/hooks"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Installing Git hooks..."

# Copier tous les hooks
cp "$SCRIPT_DIR/hooks/"* "$HOOKS_DIR/"
chmod +x "$HOOKS_DIR/"*

echo "✓ Git hooks installed successfully!"
