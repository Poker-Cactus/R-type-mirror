# Guide: Créer un Nouveau Jeu avec l'Engine R-Type

## 📖 Table des Matières
- [Aperçu](#aperçu)
- [Étape 1: Architecture de Base](#étape-1-architecture-de-base)
- [Étape 2: Définir les Composants](#étape-2-définir-les-composants)
- [Étape 3: Implémenter les Systèmes](#étape-3-implémenter-les-systèmes)
- [Étape 4: Intégrer Client/Serveur](#étape-4-intégrer-clientserveur)
- [Étape 5: Tester et Déployer](#étape-5-tester-et-déployer)
- [Exemple Complet: Mini-Jeu](#exemple-complet-mini-jeu)

---

## Aperçu

La création d'un jeu avec l'engine R-Type suit ce processus:

```
1. Analyser le jeu         (Quels sont les éléments?)
2. Concevoir les composants (Quelles données?)
3. Implémenter les systèmes (Quelle logique?)
4. Architecture réseau       (Client/Serveur)
5. Tester et intégrer        (Valider tout fonctionne)
```

---

## Étape 1: Architecture de Base

### Comprendre la Structure

L'engine utilise une **séparation Client/Serveur**:

```
┌─────────────────────────────────────────────────────────┐
│                        SERVEUR                          │
│ - Logique du jeu (systèmes principaux)                  │
│ - Autorité de l'état (vérité unique)                    │
│ - Gestion des joueurs multiples                         │
│ - Synchronisation d'état                                │
└─────────────────────────────────────────────────────────┘
         ↑                                    ↑
    Commandes                          État du monde
    (Inputs)                          (Entités/Composants)
         ↓                                    ↓
┌─────────────────────────────────────────────────────────┐
│                      CLIENT                             │
│ - Rendu graphique                                        │
│ - Gestion des inputs                                     │
│ - Affichage de l'état serveur                            │
│ - Prédiction côté client (optionnel)                     │
└─────────────────────────────────────────────────────────┘
```

### Dossiers Recommandés

```
mon-jeu/
├── include/
│   ├── components/     # Structures de données
│   ├── server/
│   │   └── systems/    # Systèmes serveur
│   └── client/
│       └── systems/    # Systèmes client
├── src/
│   ├── components/
│   ├── server/
│   │   └── systems/
│   └── client/
│       └── systems/
└── CMakeLists.txt
```

---

## Étape 2: Définir les Composants

Les composants décrivent ce qu'**est** une entité, pas ce qu'elle **fait**.

### Exemple: Jeu de Plateforme 2D

```cpp
// include/components/Core.hpp

#pragma once

#include <cstdint>

// Positionnement
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

// Mouvement
struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

// Accélération (gravité)
struct Acceleration {
    float ax = 0.0f;
    float ay = 9.81f;  // Gravité
};

// Santé
struct Health {
    int32_t hp = 100;
    int32_t maxHp = 100;
};

// Dégâts infligés
struct Damage {
    int32_t amount = 10;
};

// Boîte de collision
struct Collider {
    float width = 32.0f;
    float height = 32.0f;
    bool isSolid = true;
};

// Sprite/Affichage
struct Sprite {
    std::string texturePath;
    float width = 32.0f;
    float height = 32.0f;
};

// Marqueurs (tags)
struct Player {};      // C'est le joueur
struct Enemy {};       // C'est un ennemi
struct Projectile {};  // C'est un projectile
struct Platform {};    // C'est une plateforme
```

### Principes de Conception

✅ **Chaque composant = une responsabilité**
```cpp
struct Position { float x, y; };      // Où
struct Velocity { float vx, vy; };    // Vitesse
struct Health { int hp; };            // Points de vie
```

❌ **Pas un composant fourre-tout**
```cpp
struct GameObject {
    float x, y, vx, vy;
    int hp, maxHp;
    std::string name;
    // ... 20 autres champs
};
```

✅ **Composants légers et sérialisables**
```cpp
struct Position { float x, y; };     // 8 bytes, sérialisable

// Pas de pointers, pas de std::string si pas nécessaire
struct Player {
    int32_t id;
    std::string username;  // OK pour joueur, moins pour ennemi
};
```

---

## Étape 3: Implémenter les Systèmes

Un système contient la **logique métier**.

### Système 1: Physics (Mouvement + Gravité)

```cpp
// include/server/systems/PhysicsSystem.hpp

#pragma once

#include "ecs/ISystem.hpp"
#include "components/Core.hpp"

class PhysicsSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Velocity>());
        // Acceleration optionnel - on la vérifiera dans update
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& pos = mgr.getComponent<Position>(entity);
            auto& vel = mgr.getComponent<Velocity>(entity);

            // Appliquer accélération (gravité)
            if (mgr.hasComponent<Acceleration>(entity)) {
                auto& acc = mgr.getComponent<Acceleration>(entity);
                vel.vy += acc.ay * deltaTime;  // v = v + at
            }

            // Appliquer vélocité à position
            pos.x += vel.vx * deltaTime;      // x = x + vt
            pos.y += vel.vy * deltaTime;      // y = y + vt

            // Limiter vitesse (terminal velocity)
            if (vel.vy > 500.0f) vel.vy = 500.0f;
        }
    }
};
```

### Système 2: Collision

```cpp
// include/server/systems/CollisionSystem.hpp

#pragma once

#include "ecs/ISystem.hpp"
#include "components/Core.hpp"
#include "events/CollisionEvent.hpp"

class CollisionSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Collider>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto& eventBus = world.getEventBus();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Détection brute-force O(n²)
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto e1 = entities[i];
                auto e2 = entities[j];

                if (checkAABB(mgr, e1, e2)) {
                    // Émettre événement
                    eventBus.emit<CollisionEvent>(e1, e2);

                    // Résoudre collision (si solide)
                    resolveCollision(mgr, e1, e2);
                }
            }
        }
    }

private:
    bool checkAABB(ecs::ComponentManager& mgr, ecs::Entity e1, ecs::Entity e2) {
        auto& p1 = mgr.getComponent<Position>(e1);
        auto& c1 = mgr.getComponent<Collider>(e1);
        auto& p2 = mgr.getComponent<Position>(e2);
        auto& c2 = mgr.getComponent<Collider>(e2);

        float left1 = p1.x - c1.width / 2;
        float right1 = p1.x + c1.width / 2;
        float top1 = p1.y - c1.height / 2;
        float bottom1 = p1.y + c1.height / 2;

        float left2 = p2.x - c2.width / 2;
        float right2 = p2.x + c2.width / 2;
        float top2 = p2.y - c2.height / 2;
        float bottom2 = p2.y + c2.height / 2;

        return !(right1 < left2 || left1 > right2 ||
                 bottom1 < top2 || top1 > bottom2);
    }

    void resolveCollision(ecs::ComponentManager& mgr, ecs::Entity e1, ecs::Entity e2) {
        // Push entities apart (simple solution)
        auto& p1 = mgr.getComponent<Position>(e1);
        auto& p2 = mgr.getComponent<Position>(e2);
        auto& c1 = mgr.getComponent<Collider>(e1);
        auto& c2 = mgr.getComponent<Collider>(e2);

        float overlap_x = (c1.width + c2.width) / 2 - std::abs(p1.x - p2.x);
        float overlap_y = (c1.height + c2.height) / 2 - std::abs(p1.y - p2.y);

        if (overlap_x < overlap_y) {
            // Collision horizontale
            if (p1.x < p2.x) {
                p1.x -= overlap_x / 2;
                p2.x += overlap_x / 2;
            } else {
                p1.x += overlap_x / 2;
                p2.x -= overlap_x / 2;
            }
            if (mgr.hasComponent<Velocity>(e1)) mgr.getComponent<Velocity>(e1).vx = 0;
            if (mgr.hasComponent<Velocity>(e2)) mgr.getComponent<Velocity>(e2).vx = 0;
        } else {
            // Collision verticale
            if (p1.y < p2.y) {
                p1.y -= overlap_y / 2;
                p2.y += overlap_y / 2;
            } else {
                p1.y += overlap_y / 2;
                p2.y -= overlap_y / 2;
            }
            if (mgr.hasComponent<Velocity>(e1)) mgr.getComponent<Velocity>(e1).vy = 0;
            if (mgr.hasComponent<Velocity>(e2)) mgr.getComponent<Velocity>(e2).vy = 0;
        }
    }
};
```

### Système 3: Combat

```cpp
// include/server/systems/CombatSystem.hpp

#pragma once

#include "ecs/ISystem.hpp"
#include "components/Core.hpp"
#include "events/CollisionEvent.hpp"

class CombatSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Health>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto& eventBus = world.getEventBus();

        // Écouter les collisions
        eventBus.subscribe<CollisionEvent>(
            [&](const CollisionEvent& event) {
                ecs::Entity attacker = event.e1;
                ecs::Entity target = event.e2;

                // Si e1 est projectile et e2 peut prendre dégâts
                if (mgr.hasComponent<Projectile>(attacker) &&
                    mgr.hasComponent<Health>(target) &&
                    mgr.hasComponent<Damage>(attacker)) {
                    
                    int dmg = mgr.getComponent<Damage>(attacker).amount;
                    mgr.getComponent<Health>(target).hp -= dmg;

                    // Projectile utilisé
                    world.destroyEntity(attacker);
                }
            }
        );

        // Supprimer entités mortes
        auto entities = mgr.getEntitiesWithSignature(getSignature());
        for (auto entity : entities) {
            if (mgr.getComponent<Health>(entity).hp <= 0) {
                world.destroyEntity(entity);
            }
        }
    }
};
```

---

## Étape 4: Intégrer Client/Serveur

### Serveur: Main Loop

```cpp
// src/server/main.cpp

#include "ecs/World.hpp"
#include "components/Core.hpp"
#include "server/systems/PhysicsSystem.hpp"
#include "server/systems/CollisionSystem.hpp"
#include "server/systems/CombatSystem.hpp"
#include "server/systems/SpawnSystem.hpp"

int main() {
    ecs::World world;

    // Enregistrer systèmes dans l'ordre
    world.registerSystem<PhysicsSystem>();
    world.registerSystem<CollisionSystem>();
    world.registerSystem<CombatSystem>();
    world.registerSystem<SpawnSystem>();

    // Créer entités initiales
    auto player = world.createEntity();
    world.addComponent(player, Position{400.0f, 300.0f});
    world.addComponent(player, Velocity{0.0f, 0.0f});
    world.addComponent(player, Health{100, 100});
    world.addComponent(player, Collider{32.0f, 32.0f, true});
    world.addComponent(player, Player{});

    // Boucle de jeu
    float deltaTime = 1.0f / 60.0f;  // 60 FPS
    bool running = true;

    while (running) {
        world.update(deltaTime);
        
        // Synchroniser avec clients
        syncStateWithClients(world);
    }

    return 0;
}
```

### Client: Rendu

```cpp
// include/client/systems/RenderSystem.hpp

#pragma once

#include "ecs/ISystem.hpp"
#include "components/Core.hpp"
#include <SDL2/SDL.h>

class RenderSystem : public ecs::ISystem {
public:
    RenderSystem(SDL_Renderer* renderer) : m_renderer(renderer) {}

    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Sprite>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Nettoyer l'écran
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);

        // Dessiner toutes les entités
        for (auto entity : entities) {
            auto& pos = mgr.getComponent<Position>(entity);
            auto& sprite = mgr.getComponent<Sprite>(entity);

            SDL_Rect rect{
                static_cast<int>(pos.x - sprite.width / 2),
                static_cast<int>(pos.y - sprite.height / 2),
                static_cast<int>(sprite.width),
                static_cast<int>(sprite.height)
            };
            
            // Afficher la texture
            drawSprite(sprite, rect);
        }

        SDL_RenderPresent(m_renderer);
    }

private:
    SDL_Renderer* m_renderer;

    void drawSprite(const Sprite& sprite, const SDL_Rect& rect) {
        // TODO: Implémenter affichage texture
    }
};
```

---

## Étape 5: Tester et Déployer

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)

# Dépendances
find_package(engineCore REQUIRED)
find_package(SDL2 REQUIRED)

# Serveur
add_executable(my_game_server
    src/server/main.cpp
    src/server/systems/PhysicsSystem.cpp
    src/server/systems/CollisionSystem.cpp
)

target_link_libraries(my_game_server
    engineCore::engineCore
)

target_include_directories(my_game_server PRIVATE include)

# Client
add_executable(my_game_client
    src/client/main.cpp
    src/client/systems/RenderSystem.cpp
)

target_link_libraries(my_game_client
    engineCore::engineCore
    SDL2::SDL2
)

target_include_directories(my_game_client PRIVATE include)
```

### Tests

```cpp
// tests/test_physics.cpp

#include <gtest/gtest.h>
#include "ecs/World.hpp"
#include "components/Core.hpp"
#include "server/systems/PhysicsSystem.hpp"

TEST(PhysicsSystem, UpdatesPositionWithVelocity) {
    ecs::World world;
    world.registerSystem<PhysicsSystem>();

    auto entity = world.createEntity();
    world.addComponent(entity, Position{0.0f, 0.0f});
    world.addComponent(entity, Velocity{10.0f, 0.0f});

    float deltaTime = 1.0f;
    world.update(deltaTime);

    auto& pos = world.getComponentManager().getComponent<Position>(entity);
    EXPECT_FLOAT_EQ(pos.x, 10.0f);
}
```

---

## Exemple Complet: Mini-Jeu

### Concept: Platformer Simple

```cpp
// Le joueur saute et doit éviter les ennemis

// Étape 1: Composants
struct Position { float x, y; };
struct Velocity { float vx, vy; };
struct Acceleration { float ax, ay = 9.81f; };
struct Health { int hp = 100; };
struct Collider { float w = 32, h = 32; };
struct Sprite { std::string path; };
struct Player {};
struct Enemy {};
struct Platform {};

// Étape 2: Systèmes
class PhysicsSystem { /* ... */ };
class CollisionSystem { /* ... */ };
class CombatSystem { /* ... */ };

// Étape 3: Main
int main() {
    ecs::World world;
    world.registerSystem<PhysicsSystem>();
    world.registerSystem<CollisionSystem>();
    world.registerSystem<CombatSystem>();

    // Créer joueur
    auto player = world.createEntity();
    world.addComponent(player, Position{400, 600});
    world.addComponent(player, Velocity{0, 0});
    world.addComponent(player, Acceleration{0, 9.81});
    world.addComponent(player, Health{100, 100});
    world.addComponent(player, Collider{32, 32});
    world.addComponent(player, Sprite{"player.png"});
    world.addComponent(player, Player{});

    // Créer plateforme
    auto platform = world.createEntity();
    world.addComponent(platform, Position{400, 700});
    world.addComponent(platform, Collider{1000, 50});
    world.addComponent(platform, Platform{});

    // Boucle de jeu
    while (running) {
        world.update(0.016f);
    }
}
```

---

## Checklist: Créer un Jeu

- [ ] **Analyse** : Lister tous les éléments du jeu
- [ ] **Composants** : Définir les structures de données
- [ ] **Systèmes** : Implémenter la logique (Physics, Collision, Combat, etc.)
- [ ] **Architecture Réseau** : Client + Serveur
- [ ] **Tests** : Valider chaque système
- [ ] **Optimisation** : Profiler et améliorer
- [ ] **Documentation** : Documenter les systèmes spécifiques

L'engine R-Type fournit la base ECS; à vous de bâtir le jeu dessus! 🎮
