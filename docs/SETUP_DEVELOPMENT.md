# Development Setup

> **For basic setup**, see [Quick Start Guide](HOW_TO_START.md)

## VS Code Configuration

### Required Extensions
- **C/C++ Extension Pack**
- **CMake Tools**

### Auto-Configuration

The workspace includes portable IntelliSense config (`.vscode/c_cpp_properties.json`):
- Uses environment variables (`${env:HOME}` / `${env:USERPROFILE}`)
- Supports Mac, Linux, Windows
- Auto-detects Conan dependencies

**If IntelliSense fails:** Reload window (`Ctrl+Shift+P` → `Reload Window`)

## Development Workflow

```bash
# 1. Create feature branch
git checkout -b feat/your-feature

# 2. Develop and rebuild
./build.sh rebuild        # Fast colored rebuild
# or
cmake --build build --config Release

# 3. Test
./build/server/server
./build/client/client

# 4. Commit and push
git add . && git commit -m "[ADD/FIX] Description"
git push origin feat/your-feature
```

## Project Structure

```
engine_core/    # ECS engine
common/         # Shared components & protocols
server/         # Game server
client/         # Game client
conanfile.txt   # Dependencies (SDL2, ASIO)
```

## Dependencies

- **ASIO** 1.28.0 - Async networking
- **SDL2** 2.28.3 - Graphics rendering

## Troubleshooting

**Compiler version:** C++20 required (GCC 10+, Clang 12+, MSVC 2019+)

**Clean rebuild:**
```bash
rm -rf build CMakeUserPresets.json
conan install . --output-folder=build --build=missing --profile=conan_profile
cmake --preset conan-release && cmake --build build --config Release
```
