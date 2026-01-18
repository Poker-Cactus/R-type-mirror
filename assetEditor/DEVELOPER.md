# Asset Editor - Developer Guide

## Code Organization

### Namespace
All editor code is encapsulated in the `AssetEditor` namespace to avoid conflicts.

### Module Structure

**EditorState** (`EditorState.h/cpp`)
- Global state management (`g_state`)
- File I/O operations
- Editor mode enum

**JsonEditor** (`JsonEditor.h/cpp`)
- JSON value editing widgets
- Recursive object/array rendering
- Smart label detection for arrays

**MainMenu** (`MainMenu.h/cpp`)
- Main menu UI with centered layout
- Mode selection buttons
- Exit functionality

**Style** (`Style.h/cpp`)
- macOS-inspired dark theme
- Font loading with fallback chain
- Inter font (bundled) → macOS SF fonts → Default

**Main** (`main.cpp`)
- SDL2 initialization
- ImGui setup
- Main event loop
- Keyboard shortcuts

## Coding Standards

### Documentation
- Use Doxygen comments (`/**  */`) for public APIs
- Include `@brief`, `@param`, `@return` where applicable
- Keep comments concise and meaningful

### Style
- 4-space indentation
- Opening braces on same line
- PascalCase for functions, camelCase for variables
- Prefer `const` and references where possible

### Example
```cpp
/**
 * @brief Load a JSON file into the editor
 * @param filename Name of the file (without path)
 * @return true if loaded successfully
 */
bool LoadFile(const std::string& filename);
```

## Adding New Features

### New Editor Mode
1. Add enum value to `EditorMode` in `EditorState.h`
2. Create header/cpp pair (e.g., `SpriteEditor.h/cpp`)
3. Add render function to main switch in `main.cpp`
4. Add button to `MainMenu.cpp`

### New Widget Type
1. Add to `EditJsonValue()` in `JsonEditor.cpp`
2. Follow existing pattern: check type, render widget, mark modified

## macOS Colors Reference

```cpp
Blue:   #0A84FF  ImVec4(0.04f, 0.52f, 1.00f, 1.00f)
Green:  #30D158  ImVec4(0.19f, 0.82f, 0.35f, 1.00f)
Orange: #FF9F0A  ImVec4(1.00f, 0.62f, 0.04f, 1.00f)
Red:    #FF453A  ImVec4(1.00f, 0.27f, 0.23f, 1.00f)
```

## Building

```bash
# From project root
./build.sh editor                    # Build + run
cmake --build build --target assetEditor

# Clean build
cd build/assetEditor && make clean && make assetEditor -j8
```

## Testing

- Test on macOS and Linux (font fallback)
- Verify JSON parsing with malformed files
- Check High DPI displays (no mouse offset)
- Test keyboard shortcuts (Cmd+S/Ctrl+S)

## Dependencies

- **Dear ImGui**: v1.90.1 (vendored in `vendor/imgui/`)
- **SDL2**: Window management and rendering
- **nlohmann/json**: JSON parsing (via Conan)
- **Inter font**: Bundled in `vendor/fonts/`
