# Comment Démarrer le Projet R-Type

Guide rapide pour configurer et compiler le projet R-Type.

## 📋 Prérequis

- **CMake** >= 3.15
- **Compilateur C++20** (GCC >= 10, Clang >= 12, ou MSVC 2019+)
- **Python** (pour Conan)

## 🚀 Installation Rapide

### Ubuntu/Debian

```bash
# Installation des outils
sudo apt update && sudo apt install -y build-essential cmake python3 python3-pip

# Installation de Conan
pip install conan
conan profile detect --force
```

### macOS

```bash
# Installation des outils (nécessite Homebrew)
brew install cmake python

# Installation de Conan
pip install conan
conan profile detect --force
```

### Windows

1. Installez **Visual Studio 2019+** avec "Développement Desktop C++"
2. Installez **CMake** : [cmake.org/download](https://cmake.org/download/)
3. Installez **Python** : [python.org](https://www.python.org/)
4. Ouvrez PowerShell et exécutez :

```powershell
pip install conan
conan profile detect --force
```

## ⚙️ Compilation

À la racine du projet, exécutez :

```bash
# Installation des dépendances (SDL2, ASIO)
conan install . --output-folder=build --build=missing --profile=conan_profile

# Configuration CMake
cmake --preset conan-release

# Compilation
cmake --build build/build/Release
```

## 🎮 Exécution

### Lancer le serveur

```bash
./build/build/Release/server/server
```

### Lancer le client

```bash
./build/build/Release/client/client
```

## 🔧 En cas de problème

### Nettoyer et recommencer

```bash
rm -rf build
conan install . --output-folder=build --build=missing --profile=conan_profile
cmake --preset conan-release
cmake --build build/build/Release
```

### Erreurs d'include dans VS Code

Rechargez la fenêtre : `Cmd+Shift+P` (ou `Ctrl+Shift+P`) → "Reload Window"

## 📚 Documentation

- [Configuration de développement](SETUP_DEVELOPMENT.md) - Guide détaillé
- [Architecture du projet](ARCHITECTURE.md) - Structure du code

