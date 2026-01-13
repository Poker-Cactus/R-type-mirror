#!/bin/bash
# Script to download ImGui sources for the Asset Editor

set -e

IMGUI_VERSION="v1.90.1"
IMGUI_DIR="$(dirname "$0")/vendor/imgui"

if [ -d "$IMGUI_DIR" ]; then
    echo "ImGui already present at $IMGUI_DIR"
    exit 0
fi

echo "Downloading ImGui $IMGUI_VERSION..."

mkdir -p "$IMGUI_DIR"
mkdir -p "$IMGUI_DIR/backends"

# Download core ImGui files
IMGUI_BASE_URL="https://raw.githubusercontent.com/ocornut/imgui/$IMGUI_VERSION"

curl -sL "$IMGUI_BASE_URL/imgui.h" -o "$IMGUI_DIR/imgui.h"
curl -sL "$IMGUI_BASE_URL/imgui.cpp" -o "$IMGUI_DIR/imgui.cpp"
curl -sL "$IMGUI_BASE_URL/imgui_internal.h" -o "$IMGUI_DIR/imgui_internal.h"
curl -sL "$IMGUI_BASE_URL/imgui_demo.cpp" -o "$IMGUI_DIR/imgui_demo.cpp"
curl -sL "$IMGUI_BASE_URL/imgui_draw.cpp" -o "$IMGUI_DIR/imgui_draw.cpp"
curl -sL "$IMGUI_BASE_URL/imgui_tables.cpp" -o "$IMGUI_DIR/imgui_tables.cpp"
curl -sL "$IMGUI_BASE_URL/imgui_widgets.cpp" -o "$IMGUI_DIR/imgui_widgets.cpp"
curl -sL "$IMGUI_BASE_URL/imstb_rectpack.h" -o "$IMGUI_DIR/imstb_rectpack.h"
curl -sL "$IMGUI_BASE_URL/imstb_textedit.h" -o "$IMGUI_DIR/imstb_textedit.h"
curl -sL "$IMGUI_BASE_URL/imstb_truetype.h" -o "$IMGUI_DIR/imstb_truetype.h"
curl -sL "$IMGUI_BASE_URL/imconfig.h" -o "$IMGUI_DIR/imconfig.h"

# Download SDL2 backend files
curl -sL "$IMGUI_BASE_URL/backends/imgui_impl_sdl2.h" -o "$IMGUI_DIR/backends/imgui_impl_sdl2.h"
curl -sL "$IMGUI_BASE_URL/backends/imgui_impl_sdl2.cpp" -o "$IMGUI_DIR/backends/imgui_impl_sdl2.cpp"
curl -sL "$IMGUI_BASE_URL/backends/imgui_impl_sdlrenderer2.h" -o "$IMGUI_DIR/backends/imgui_impl_sdlrenderer2.h"
curl -sL "$IMGUI_BASE_URL/backends/imgui_impl_sdlrenderer2.cpp" -o "$IMGUI_DIR/backends/imgui_impl_sdlrenderer2.cpp"

echo "ImGui $IMGUI_VERSION downloaded successfully!"
