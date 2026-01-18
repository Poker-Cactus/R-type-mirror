# Wiki R-Type

Bienvenue sur la documentation du projet R-Type - Un moteur de jeu multi-joueurs basé sur l'architecture ECS.

## 📚 Table des Matières

### Pour Commencer
- **[Guide de Démarrage Rapide](QUICKSTART.md)** - Installation et compilation rapide
- **[Architecture du Projet](ARCHITECTURE.md)** - Structure détaillée et modules
- **[Résumé Architecture](GAME_ARCHITECTURE.md)** - Vue d'ensemble technique
- **[Comment Débuter](HOW_TO_START.md)** - Guide complet pour les nouveaux contributeurs

### 🎮 Créer un Jeu avec R-Type

> **Nouveau développeur?** Commencez par ces guides:

- **[Créer un Nouveau Jeu](guides/Creating-A-Game.md)** ⭐ **START HERE**
  - Vue d'ensemble du processus
  - Exemple complet pas à pas
  - Architecture d'un jeu simple
  
- **[Comprendre l'ECS](technical/ECS-Engine.md)** 📚 Fondamental
  - Concepts d'Entity-Component-System
  - Architecture ECS détaillée
  - Exemples pratiques et patterns
  
- **[Créer des Composants](guides/Adding-Components.md)** 🧩
  - Structure des composants
  - Signatures et filtering
  - Sérialisation réseau
  
- **[Implémenter les Systèmes](guides/Creating-Systems.md)** ⚙️
  - Architecture d'un système
  - Systèmes de base (physics, collision, combat)
  - Patterns avancés

### Architecture & Best Practices

- **[Architecture & Best Practices](guides/Architecture-Best-Practices.md)** 🏗️
  - Patterns de conception (Factory, Pool, Events)
  - Client vs Serveur
  - Performance & optimisation
  - Bonnes pratiques et anti-patterns

### Documentation Technique Complète

- **[ECS Engine (Détail)](technical/ECS-Engine.md)** - Deep dive sur l'ECS
- **[Systèmes du Moteur](technical/SYSTEM_MANAGER.md)** - Gestion des systèmes
- **[Protocole Réseau](PROTOCOL.md)** - Communication client-serveur
- **[Sprites Côté Serveur](SERVER_DRIVEN_SPRITES.md)** - Gestion des sprites

### Code & Standards

- **[Nomenclature](NOMENCLATURE.md)** - Conventions de nommage
- **[Coding Standards](CODING_STANDARDS.md)** - Standards de code
- **[Configuration Windows MSVC](WINDOWS_MSVC_COMPLIANCE.md)** - Windows setup

## 🚀 Démarrage Rapide

```bash
# Installation de Conan
pipx install conan  # Ubuntu/Debian
pip install conan   # macOS/Windows

# Compilation
rm -rf build && mkdir build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 🎯 Structure du Projet

```
rtype/
├── engineCore/    # Moteur ECS générique
├── common/         # Composants et protocole partagés
├── server/         # Serveur de jeu
├── client/         # Client de jeu
└── docs/           # Documentation (ce wiki)
```

## 🛠️ Technologies

| Composant | Technologie |
|-----------|-------------|
| Langage   | C++20       |
| Build     | CMake + Conan |
| Graphisme | SDL2        |
| Réseau    | Asio        |

## 📖 Liens Utiles

- [Dépôt GitHub](https://github.com/Poker-Cactus/R-type-mirror)
- [Issues](https://github.com/Poker-Cactus/R-type-mirror/issues)
- [Pull Requests](https://github.com/Poker-Cactus/R-type-mirror/pulls)

## 🤝 Contribution

Consultez le [guide de contribution](guides/Contributing.md) pour savoir comment participer au projet.

---

**EPITECH PROJECT - 2025**
