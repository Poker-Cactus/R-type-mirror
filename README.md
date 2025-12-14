# R-Type

Multiplayer networked game engine using ECS architecture, inspired by the classic R-Type.

## 🚀 Quick Start

### Linux/macOS

```bash
# Install Git hooks (format code automatically on commit)
./scripts/install-hooks.sh

# Build with beautiful colors 🎨
./build.sh

# Or manually:
conan install . --output-folder=build --build=missing --profile=conan_profile
cmake --preset conan-release
cmake --build build --config Release

# Run
./build/server/server
./build/client/client
```

### Windows (MSVC)

```powershell
# Install dependencies with Conan
python -m conans.conan install . --output-folder=build --build=missing -s build_type=Release

# Configure with CMake (uses Visual Studio generator by default)
cmake --preset conan-default

# Build
cmake --build build --config Release

# Copy required DLL next to client executable
Copy-Item -Path "build\libs\Release\sdl2_module.dll" -Destination "build\client\Release\" -Force

# Run (from project root)
.\build\server\Release\server.exe
.\build\client\Release\client.exe
```

## 📚 Documentation

- [Quick Start Guide](docs/HOW_TO_START.md) - Build & run instructions
- [Development Setup](docs/SETUP_DEVELOPMENT.md) - VS Code & workflow
- [Architecture](docs/ARCHITECTURE.md) - Project structure
- [Coding Standards](docs/CODING_STANDARDS.md) - Code formatting & linting rules

## 🛠️ Stack

**C++20** • **CMake** • **Conan** • **SDL2** • **ASIO**

### ✅ Compiler Support

- **Linux**: GCC 10+, Clang 12+
- **Windows**: MSVC 2019+ (Visual Studio 2019/2022)
- **macOS**: Clang 12+ (Xcode 13+)

### 🔧 Cross-Platform Features

- ✅ Threading: `std::thread`, `std::mutex`, `std::atomic`
- ✅ Networking: ASIO (no raw sockets, no POSIX dependencies)
- ✅ Graphics: SDL2 (cross-platform)
- ✅ Build System: CMake with MSVC and GCC/Clang support

---

EPITECH PROJECT © 2025
