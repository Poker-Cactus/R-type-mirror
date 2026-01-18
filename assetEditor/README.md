# R-Type Asset Editor

Standalone ImGui-based editor for R-Type game configuration files.

## Architecture

```
assetEditor/
├── include/              # Header files
│   ├── EditorState.h    # Global state and file operations
│   ├── JsonEditor.h     # JSON editing functionality
│   ├── MainMenu.h       # Main menu interface
│   └── Style.h          # ImGui styling and fonts
├── src/                 # Implementation files
│   ├── main.cpp         # Application entry point
│   ├── EditorState.cpp
│   ├── JsonEditor.cpp
│   ├── MainMenu.cpp
│   └── Style.cpp
└── vendor/              # Third-party dependencies
    ├── imgui/           # ImGui v1.90.1 (vendored)
    └── fonts/           # Bundled Inter font
```

## Features

- **JSON Config Editor**: Edit game configuration files (enemies, levels, etc.)
- **Smart Array Detection**: Automatically detects and displays main arrays
- **macOS Dark Theme**: Rounded corners, system accent colors (#0A84FF blue, #30D158 green)
- **Bundled Font**: Inter font for cross-platform consistency
- **Keyboard Shortcuts**: Cmd+S / Ctrl+S to save

## Building

```bash
./build.sh editor           # Build and run
cmake --build build --target assetEditor
```

## Usage

1. Launch the editor
2. Select "JSON Config Editor" from main menu
3. Choose a file from the left panel
4. Edit values in the right panel
5. Save with the button or Cmd+S

## Technical Details

- **Language**: C++20
- **UI Framework**: Dear ImGui 1.90.1 + SDL2
- **JSON Library**: nlohmann/json 3.11.2
- **Font**: Inter Regular 16px (OTF, 3x3 oversampling)
- **Build System**: CMake + Conan

## Future Features

- Sprite Editor (coming soon)
- Undo/Redo system
- Multi-file editing
