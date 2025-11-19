# Architecture du Projet R-Type

## 📁 Structure Générale

```
rtype/
├── engine_core/          # Moteur ECS générique
├── common/               # Code partagé (composants, protocole)
├── server/               # Application serveur
├── client/               # Application client
└── docs/                 # Documentation
```

## 🎯 Modules

### 1. **engine_core** (Bibliothèque Statique)
Moteur ECS (Entity Component System) générique et réutilisable.

```
engine_core/
├── include/
│   ├── ecs/              # Système ECS (Registry, Entity, System)
│   └── utils/            # Utilitaires (Logger, Timer, etc.)
└── src/
    ├── ecs/
    └── utils/
```

**Responsabilités :**
- Gestion des entités et composants
- Vues et itération sur les entités
- Système de base pour la logique métier

---

### 2. **common** (Bibliothèque Statique)
Code partagé entre le client et le serveur.

```
common/
├── include/
│   ├── components/       # Définitions des composants (Position, Velocity, Health, etc.)
│   └── protocol/         # Structures de paquets réseau
└── src/
    ├── components/
    └── protocol/
```

**Responsabilités :**
- Composants du jeu (structs pures)
- Protocole réseau binaire
- Sérialisation/Désérialisation

---

### 3. **server** (Exécutable)
Serveur de jeu avec logique métier et gestion réseau.

```
server/
├── include/
│   ├── systems/          # Systèmes de logique (Collision, Movement, Spawn)
│   └── network/          # Gestionnaire réseau Asio
└── src/
    ├── systems/
    ├── network/
    └── main.cpp          # Entry point
```

**Responsabilités :**
- Boucle de jeu principale
- Systèmes de logique métier
- Gestion réseau multi-threadée (Asio)
- Autorité sur l'état du jeu

---

### 4. **client** (Exécutable)
Client de jeu avec rendu graphique et gestion des inputs.

```
client/
├── include/
│   ├── rendering/        # Système de rendu SDL2
│   └── input/            # Gestionnaire d'input (clavier, souris)
└── src/
    ├── rendering/
    ├── input/
    └── main.cpp          # Entry point
```

**Responsabilités :**
- Boucle de rendu graphique (SDL2)
- Gestion des événements utilisateur
- Interpolation et prédiction client-side
- Interface utilisateur

---

## 🔗 Dépendances Entre Modules

```
server ──┬──> engine_core
         └──> common

client ──┬──> engine_core
         └──> common

common ──> asio (protocole réseau)
```

## 🛠️ Technologies

| Module       | Dépendances              |
|--------------|--------------------------|
| engine_core  | C++20                    |
| common       | C++20, Asio              |
| server       | Asio (réseau)            |
| client       | SDL2 (graphisme)         |

## 📝 Conventions de Nommage

- **Fichiers** : PascalCase (ex: `Registry.hpp`, `Components.hpp`)
- **Classes** : PascalCase (ex: `class Registry`)
- **Fonctions/Méthodes** : camelCase (ex: `createEntity()`)
- **Variables** : camelCase avec préfixe `_` pour les membres privés (ex: `_nextEntityId`)
- **Namespaces** : PascalCase (ex: `namespace ECS`, `namespace Components`)

## 🚀 Compilation

Voir `docs/HOW_TO_START.md` pour les instructions détaillées.
