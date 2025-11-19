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

- **[🚀 Guide de Démarrage →](docs/HOW_TO_START.md)** - Instructions d'installation, configuration et compilation
- **[🏗️ Architecture →](docs/ARCHITECTURE.md)** - Structure détaillée et diagramme des modules

## 🏗️ Structure du Projet

```
rtype/
├── engine_core/      # Moteur ECS générique
├── common/           # Composants et protocole partagés
├── server/           # Serveur de jeu (logique + réseau)
├── client/           # Client de jeu (rendu + input)
└── docs/             # Documentation
```

## 🛠️ Technologies

| Composant | Technologie   |
| --------- | ------------- |
| Langage   | C++20         |
| Graphisme | SDL2          |
| Réseau    | Asio          |
| Build     | CMake + Conan |

## 📝 Licence

EPITECH PROJECT - 2025

## 👥 Contributeurs

Voir la liste des contributeurs dans l'historique Git.
