# R-Type

Un moteur de jeu multi-joueurs en réseau basé sur l'architecture ECS (Entity Component System), inspiré du jeu classique R-Type.

## 🎮 À propos

Ce projet est un moteur de jeu cross-platform (Linux/macOS/Windows) développé en C++20, utilisant :
- **ECS** pour l'architecture du moteur
- **SDL2** pour le rendu graphique côté client
- **Asio** pour la gestion réseau côté serveur
- **Conan** pour la gestion des dépendances
- **CMake** pour le système de build

## 📚 Documentation

- [Guide de Démarrage](docs/HOW_TO_START.md) - Instructions d'installation et de compilation
- [Architecture](ARCHITECTURE.md) - Structure détaillée du projet

## 🏗️ Structure du Projet

```
rtype/
├── engine_core/      # Moteur ECS générique
├── common/           # Composants et protocole partagés
├── server/           # Serveur de jeu (logique + réseau)
├── client/           # Client de jeu (rendu + input)
└── docs/             # Documentation
```

## 🚀 Démarrage Rapide

### Prérequis
- CMake 3.15+
- Compilateur C++20 (GCC 10+, Clang 11+, MSVC 2019+)
- Python 3 (pour Conan)

### Compilation

```bash
# Installation de Conan
pip install conan
conan profile detect --force

# Build
rm -rf build ; mkdir build
cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Exécution

```bash
# Serveur
./server/server

# Client
./client/client
```

## 🛠️ Technologies

| Composant | Technologie |
|-----------|-------------|
| Langage   | C++20       |
| Graphisme | SDL2        |
| Réseau    | Asio        |
| Build     | CMake + Conan |

## 📝 Licence

EPITECH PROJECT - 2025

## 👥 Contributeurs

Voir la liste des contributeurs dans l'historique Git.
