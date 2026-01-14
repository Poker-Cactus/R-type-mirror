# 🚀 R-Type Quickstart Guide

Guide rapide pour installer et lancer R-Type sur un nouveau PC.

---

## 📋 Prérequis

| Outil | Version | Description |
|-------|---------|-------------|
| **CMake** | 3.15+ | Build system |
| **C++ Compiler** | C++20 (GCC 10+, Clang 12+, MSVC 2019+) | Compilateur |
| **Python** | 3.8+ | Pour Conan |
| **Conan** | 2.0+ | Package manager C++ |
| **Git** | 2.0+ | Version control |

---

## 🐧 Linux (Ubuntu/Debian)

### 1. Installer les dépendances système

```bash
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    pkg-config \
    libsfml-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libudev-dev \
    libopenal-dev \
    libvorbis-dev \
    libflac-dev
```

### 2. Installer Conan

```bash
pip3 install conan
conan profile detect --force
```

### 3. Cloner et compiler

```bash
git clone git@github.com:Poker-Cactus/R-type-mirror.git
cd R-type-mirror
./build.sh
```

---

## 🍎 macOS

### 1. Installer Homebrew (si pas déjà installé)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. Installer les dépendances

```bash
brew install cmake python3 sfml
```

### 3. Installer Conan

```bash
pip3 install conan
conan profile detect --force
```

### 4. Cloner et compiler

```bash
git clone git@github.com:Poker-Cactus/R-type-mirror.git
cd R-type-mirror
./build.sh
```

---

## 🪟 Windows

### 1. Installer les outils

1. **Visual Studio 2022** (ou 2019)
   - Télécharger depuis [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - Installer le workload **"Desktop development with C++"**

2. **CMake**
   - Télécharger depuis [cmake.org](https://cmake.org/download/)
   - Cocher "Add CMake to PATH" pendant l'installation

3. **Python 3**
   - Télécharger depuis [python.org](https://www.python.org/downloads/)
   - Cocher "Add Python to PATH" pendant l'installation

4. **Git**
   - Télécharger depuis [git-scm.com](https://git-scm.com/download/win)

### 2. Installer Conan (PowerShell en admin)

```powershell
pip install conan
conan profile detect --force
```

### 3. Cloner et compiler (PowerShell)

```powershell
git clone git@github.com:Poker-Cactus/R-type-mirror.git
cd R-type-mirror

# Installer les dépendances Conan
conan install . --output-folder=build --build=missing -s build_type=Release

# Configurer CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="build/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release

# Compiler
cmake --build build --config Release
```

---

## 🎮 Lancer le jeu

### Serveur

```bash
# Linux/macOS
./build/server/server

# Windows
.\build\server\Release\server.exe
```

### Client

```bash
# Linux/macOS
./build/client/client

# Windows
.\build\client\Release\client.exe
```

### Asset Editor

```bash
# Linux/macOS
./build/assetEditor/assetEditor

# Windows
.\build\assetEditor\Release\assetEditor.exe
```

---

## 🔄 Commandes utiles

| Commande | Description |
|----------|-------------|
| `./build.sh` | Build complet (dépendances + compile) |
| `./build.sh rebuild` | Recompile uniquement (rapide) |
| `./build.sh server` | Lance le serveur |
| `./build.sh client` | Lance le client |
| `./build.sh editor` | Lance l'éditeur d'assets |
| `./build.sh clean` | Nettoie le dossier build |
| `./build.sh tests` | Lance les tests unitaires |

---

## 🐛 Troubleshooting

### Conan profile not found

```bash
conan profile detect --force
```

### CMake cache corrompu

```bash
rm -rf build/CMakeCache.txt
./build.sh
```

### SFML non trouvé (Linux)

```bash
sudo apt install libsfml-dev
```

### SFML non trouvé (macOS)

```bash
brew install sfml
```

### Erreur de linkage OpenGL (Linux)

```bash
sudo apt install libgl1-mesa-dev
```

### Pourquoi Conan ne peut pas installer les dépendances système

Conan gère des paquets C/C++ (bibliothèques et artefacts) mais ne remplace pas le gestionnaire
de paquets de la distribution. Certaines recettes (par ex. `xorg/system`) déclarent des
`system_requirements` qui correspondent à des paquets fournis par l'OS (headers, librairies
runtime, paquets X11, etc.).

Par défaut Conan est en mode "check" pour l'installation système : il vérifie la présence
de ces paquets mais n'exécute pas d'installation automatique afin d'éviter des modifications
non désirées du système. Vous devez donc installer ces paquets manuellement (ou configurer
Conan pour autoriser l'installation automatique).

Exemples :

- Installer manuellement (Linux/Ubuntu) :

```bash
sudo apt-get update && sudo apt-get install -y <liste-de-paquets-systeme>
```

- Autoriser Conan à installer les paquets (utiliser avec précaution) :

```bash
conan install . -c tools.system.package_manager:mode=install
```

Dans notre CI nous installons explicitement les paquets système avant d'exécuter `conan install`.
Voir `.github/workflows/build.yml` pour la liste et l'ordre d'installation.

### Windows: "Visual Studio not found"

Assurez-vous que le workload "Desktop development with C++" est installé dans Visual Studio Installer.

---

## 📁 Structure du projet

```
R-type-mirror/
├── build/          # Fichiers compilés
├── client/         # Code source client
├── server/         # Code source serveur
├── common/         # Code partagé
├── engineCore/     # Moteur ECS
├── network/        # Couche réseau
├── assetEditor/    # Éditeur d'assets
├── docs/           # Documentation
├── conanfile.txt   # Dépendances Conan
└── build.sh        # Script de build
```

---

## 🔗 Liens utiles

- [Architecture du projet](ARCHITECTURE.md)
- [Standards de code](CODING_STANDARDS.md)
- [Protocole réseau](PROTOCOL.md)
- [Configuration VS Code](SETUP_DEVELOPMENT.md)
