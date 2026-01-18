/**
 * @mainpage R-Type Asset Editor
 *
 * @section intro Introduction
 * Standalone ImGui-based editor for R-Type game configuration files.
 * Features JSON editing with smart array detection, macOS-styled dark theme,
 * and bundled Inter font for cross-platform consistency.
 *
 * @section architecture Architecture Overview
 *
 * The editor is organized into modular components:
 *
 * - **EditorState**: Global state management and file I/O operations
 * - **JsonEditor**: JSON value editing with recursive rendering
 * - **MainMenu**: Main menu interface with mode selection
 * - **SpriteEditor**: Pixel-level sprite editing with layers
 * - **Style**: ImGui theming and font loading
 * - **Main**: SDL2 initialization and event loop
 *
 * All code is encapsulated in the `AssetEditor` namespace.
 *
 * @section features Key Features
 *
 * - Smart JSON array detection (auto-expands main arrays)
 * - Multi-layer sprite editing with undo/redo
 * - RAII resource management (no raw pointers)
 * - macOS dark theme with system accent colors
 * - Bundled Inter font with fallback chain
 * - Keyboard shortcuts (Cmd+S / Ctrl+S to save)
 * - Drag & drop file import
 *
 * @section usage Usage
 *
 * @code
 * // Build and run
 * ./build.sh editor
 *
 * // Or build manually
 * cmake --build build --target assetEditor
 * ./build/assetEditor/assetEditor
 * @endcode
 *
 * @section dependencies Dependencies
 *
 * - **Dear ImGui** v1.90.1 (vendored)
 * - **SDL2** (via Conan)
 * - **SDL2_image** (via Conan)
 * - **nlohmann/json** 3.11.2 (via Conan)
 * - **Inter font** (bundled)
 *
 * @author R-Type Team
 * @version 1.0.0
 */
