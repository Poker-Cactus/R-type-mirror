# Getting Started with R-Type

Quick start guide to build and run the R-Type project.

---

## 📋 Prerequisites

| Requirement | Minimum Version |
|------------|-----------------|
| **CMake** | 3.15+ |
| **C++ Compiler** | C++20 support (GCC 10+, Clang 12+, MSVC 2019+) |
| **Python** | 3.6+ (for Conan package manager) |
| **Conan** | 2.0+ |

---

## 🛠️ Environment Setup

### Linux (Ubuntu/Debian)

```bash
# Install build tools and dependencies
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    python3 \
    python3-pip

# Install Conan package manager
pip3 install conan
conan profile detect --force
```

### macOS

```bash
# Install dependencies via Homebrew
brew install cmake python3

# Install Conan package manager
pip3 install conan
conan profile detect --force
```

### Windows

**Prerequisites:**
- Visual Studio 2019+ with "Desktop development with C++" workload
- CMake from [cmake.org](https://cmake.org/download/)
- Python 3 from [python.org](https://www.python.org/)

```powershell
# Install Conan package manager
pip install conan
conan profile detect --force
```

---

## 🚀 Build Instructions

Clone the repository and navigate to the project root:

```bash
git clone https://github.com/Poker-Cactus/R-type-mirror.git
cd R-type-mirror
```

### Quick Build (Linux/macOS with colors! 🎨)

```bash
./build.sh
```

### Manual Build

#### Linux/macOS

**Step 1: Install Dependencies**

```bash
conan install . --output-folder=build --build=missing --profile=conan_profile
```

This command:
- Downloads and builds project dependencies (SDL2, ASIO)
- Generates CMake configuration files
- Creates a `conan-release` preset

**Step 2: Configure Build System**

```bash
cmake --preset conan-release
```

**Step 3: Compile Project**

```bash
cmake --build build --config Release
```

**Build artifacts location:** `build/`

**Run the Application**

```bash
# Terminal 1 - Start server
./build/server/server

# Terminal 2 - Start client  
./build/client/client
```

---

#### Windows (MSVC)

**Step 1: Install Dependencies**

Open **Developer Command Prompt for VS 2022** (or VS 2019) and run:

```powershell
conan install . --output-folder=build --build=missing --profile=conan_profile
```

**Step 2: Configure Build System with Visual Studio Generator**

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
```

*Note: Use "Visual Studio 16 2019" if you have VS 2019 instead of VS 2022.*

**Step 3: Compile Project**

```powershell
cmake --build build --config Release
```

**Build artifacts location:** `build/Release/`

**Run the Application**

```powershell
# Terminal 1 - Start server
.\build\server\Release\server.exe

# Terminal 2 - Start client
.\build\client\Release\client.exe
```

---

### Alternative Windows Build (Command Line)

If you prefer a simpler command-line workflow on Windows:

```powershell
# One-liner build
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake && cmake --build build-msvc --config Release

# Run
.\build-msvc\server\Release\server.exe
.\build-msvc\client\Release\client.exe
```

## Troubleshooting

**Quick rebuild:** `./build.sh rebuild`

**Clean build:** `./build.sh clean && ./build.sh`

**Manual clean:**
```bash
rm -rf build CMakeUserPresets.json
conan install . --output-folder=build --build=missing --profile=conan_profile
cmake --preset conan-release
cmake --build build --config Release
```

**VS Code IntelliSense errors?** Reload window: `Ctrl+Shift+P` → `Reload Window`

---

📚 **More info:** [Development Setup](SETUP_DEVELOPMENT.md) • [Architecture](ARCHITECTURE.md)

