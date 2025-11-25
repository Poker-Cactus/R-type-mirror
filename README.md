# R-Type

Multiplayer networked game engine using ECS architecture, inspired by the classic R-Type.

## 🚀 Quick Start

```bash
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

## 📚 Documentation

- [Quick Start Guide](docs/HOW_TO_START.md) - Build & run instructions
- [Development Setup](docs/SETUP_DEVELOPMENT.md) - VS Code & workflow
- [Architecture](docs/ARCHITECTURE.md) - Project structure

## 🛠️ Stack

**C++20** • **CMake** • **Conan** • **SDL2** • **ASIO**

---

EPITECH PROJECT © 2025
