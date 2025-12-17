# Wiki R-Type

Bienvenue sur la documentation du projet R-Type - Un moteur de jeu multi-joueurs basé sur l'architecture ECS.

## 📚 Table des Matières

### Pour Commencer
- **[Guide de Démarrage](HOW_TO_START.md)** - Installation et compilation du projet
- **[Architecture](ARCHITECTURE.md)** - Structure détaillée du projet
- **[Résumé](SUMMARY.md)** - Vue d'ensemble de l'architecture

### Documentation Technique
- **[ECS Engine](technical/ECS-Engine.md)** - Système Entity-Component-System
- **[Composants](technical/Components.md)** - Liste des composants disponibles
- **[Protocole Réseau](technical/Network-Protocol.md)** - Communication client-serveur
- **[Systèmes](technical/Systems.md)** - Systèmes de logique de jeu

### Guides de Développement
- **[Ajouter un Composant](guides/Adding-Components.md)** - Comment créer un nouveau composant
- **[Créer un Système](guides/Creating-Systems.md)** - Comment créer un système de jeu
- **[Conventions de Code](guides/Code-Conventions.md)** - Standards et bonnes pratiques

### API Reference
- **[Registry API](api/Registry.md)** - API du Registry ECS
- **[Network API](api/Network.md)** - API réseau
- **[Rendering API](api/Rendering.md)** - API de rendu

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
