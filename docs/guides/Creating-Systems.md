# Systèmes du Moteur ECS - Guide Complet

## 📋 Table des Matières
- [Introduction](#introduction)
- [Structure d'un Système](#structure-dun-système)
- [Cycle de Vie](#cycle-de-vie)
- [Systèmes de Base](#systèmes-de-base)
- [Patterns Avancés](#patterns-avancés)
- [Optimisation](#optimisation)
- [Débogage](#débogage)

---

## Introduction

Un **système** dans l'ECS est une classe qui:
1. Hérite de `ISystem`
2. Définit une signature (quels composants elle utilise)
3. Implémente la logique dans `update()`

```cpp
class MonSysteme : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        // Quels composants ce système nécessite?
    }

    void update(ecs::World& world, float deltaTime) override {
        // Exécuté chaque frame
    }
};
```

---

## Structure d'un Système

### Template Minimum

```cpp
// include/server/systems/MonSysteme.hpp

#pragma once

#include "ecs/ISystem.hpp"
#include "components/Core.hpp"

class MonSysteme : public ecs::ISystem {
public:
    /**
     * @brief Définit la signature requise
     * 
     * La signature est un bitset qui indique quels composants
     * une entité doit avoir pour être traitée par ce système.
     */
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Velocity>());
        // Pas besoin de lister les composants optionnels!
        return sig;
    }

    /**
     * @brief Mise à jour du système
     * 
     * @param world Le monde contenant les entités
     * @param deltaTime Temps écoulé depuis le dernier frame (secondes)
     */
    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        
        // Récupérer toutes les entités avec la signature requise
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Traiter chaque entité
        for (auto entity : entities) {
            // Accéder aux composants
            auto& pos = mgr.getComponent<Position>(entity);
            auto& vel = mgr.getComponent<Velocity>(entity);

            // Appliquer la logique
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }
};
```

---

## Cycle de Vie

### Enregistrement des Systèmes

L'ordre d'enregistrement détermine **l'ordre d'exécution**:

```cpp
ecs::World world;

// L'ordre IMPORTE!
world.registerSystem<InputSystem>();        // 1. Lire inputs
world.registerSystem<MovementSystem>();     // 2. Appliquer mouvements
world.registerSystem<CollisionSystem>();    // 3. Détecter collisions
world.registerSystem<AnimationSystem>();    // 4. Mettre à jour animations
world.registerSystem<RenderSystem>();       // 5. Afficher
```

### Exécution dans la Boucle

```cpp
while (running) {
    float deltaTime = clock.getElapsedTime();

    // Appel à world.update() exécute TOUS les systèmes
    // dans l'ordre d'enregistrement
    world.update(deltaTime);

    // Après world.update(), on peut faire du post-processing
    handleNetworkMessages();
}
```

---

## Systèmes de Base

### 1. Système de Mouvement (Physics)

```cpp
class MovementSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Velocity>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& pos = mgr.getComponent<Position>(entity);
            auto& vel = mgr.getComponent<Velocity>(entity);

            // Appliquer l'accélération si présente
            if (mgr.hasComponent<Acceleration>(entity)) {
                auto& acc = mgr.getComponent<Acceleration>(entity);
                vel.vx += acc.ax * deltaTime;
                vel.vy += acc.ay * deltaTime;
            }

            // Mettre à jour position
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;

            // Limiter vitesse maximale
            float speed = std::sqrt(vel.vx * vel.vx + vel.vy * vel.vy);
            if (speed > 500.0f) {
                vel.vx = (vel.vx / speed) * 500.0f;
                vel.vy = (vel.vy / speed) * 500.0f;
            }
        }
    }
};
```

### 2. Système de Collision

```cpp
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

        // Détection brute-force
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto e1 = entities[i];
                auto e2 = entities[j];

                if (isColliding(mgr, e1, e2)) {
                    // Émettre événement de collision
                    eventBus.emit<CollisionEvent>(e1, e2);

                    // Résoudre collision physiquement
                    if (mgr.getComponent<Collider>(e1).isSolid &&
                        mgr.getComponent<Collider>(e2).isSolid) {
                        separateEntities(mgr, e1, e2);
                    }
                }
            }
        }
    }

private:
    bool isColliding(ecs::ComponentManager& mgr, ecs::Entity e1, ecs::Entity e2) {
        auto& p1 = mgr.getComponent<Position>(e1);
        auto& c1 = mgr.getComponent<Collider>(e1);
        auto& p2 = mgr.getComponent<Position>(e2);
        auto& c2 = mgr.getComponent<Collider>(e2);

        float dx = std::abs(p1.x - p2.x);
        float dy = std::abs(p1.y - p2.y);

        return dx < (c1.width + c2.width) / 2.0f &&
               dy < (c1.height + c2.height) / 2.0f;
    }

    void separateEntities(ecs::ComponentManager& mgr, ecs::Entity e1, ecs::Entity e2) {
        // Pousser les entités l'une de l'autre
        auto& p1 = mgr.getComponent<Position>(e1);
        auto& p2 = mgr.getComponent<Position>(e2);
        auto& c1 = mgr.getComponent<Collider>(e1);
        auto& c2 = mgr.getComponent<Collider>(e2);

        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist == 0) return;

        float overlap = ((c1.width + c2.width) / 2.0f - dist) / 2.0f;
        float nx = dx / dist;
        float ny = dy / dist;

        p1.x -= nx * overlap;
        p1.y -= ny * overlap;
        p2.x += nx * overlap;
        p2.y += ny * overlap;
    }
};
```

### 3. Système de Santé/Combat

```cpp
class HealthSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Health>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto& eventBus = world.getEventBus();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        std::vector<ecs::Entity> toDestroy;

        for (auto entity : entities) {
            auto& health = mgr.getComponent<Health>(entity);

            // Appliquer dégâts over-time si présents
            if (mgr.hasComponent<DamageOverTime>(entity)) {
                auto& dot = mgr.getComponent<DamageOverTime>(entity);
                health.hp -= dot.damagePerSecond * deltaTime;
                dot.duration -= deltaTime;

                if (dot.duration <= 0.0f) {
                    mgr.removeComponent<DamageOverTime>(entity);
                }
            }

            // Vérifier si mort
            if (health.hp <= 0) {
                toDestroy.push_back(entity);
                eventBus.emit<DeathEvent>(entity);
            }
        }

        // Supprimer les morts (hors de la boucle!)
        for (auto entity : toDestroy) {
            world.destroyEntity(entity);
        }
    }
};
```

### 4. Système de Spawn

```cpp
class SpawnSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        // Ce système n'a pas de signature
        // Il crée de nouvelles entités
        return ecs::ComponentSignature{};
    }

    void update(ecs::World& world, float deltaTime) override {
        m_spawnTimer += deltaTime;

        // Spawner un ennemi tous les 2 secondes
        if (m_spawnTimer >= 2.0f) {
            spawnEnemy(world);
            m_spawnTimer = 0.0f;
        }
    }

private:
    float m_spawnTimer = 0.0f;

    void spawnEnemy(ecs::World& world) {
        auto enemy = world.createEntity();
        world.addComponent(enemy, Position{0.0f, 100.0f});
        world.addComponent(enemy, Velocity{100.0f, 0.0f});
        world.addComponent(enemy, Health{30, 30});
        world.addComponent(enemy, Collider{32.0f, 32.0f});
        world.addComponent(enemy, Enemy{});
    }
};
```

### 5. Système d'Input (Client)

```cpp
class InputSystem : public ecs::ISystem {
public:
    InputSystem(InputManager* inputMgr) : m_inputMgr(inputMgr) {}

    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Player>());
        sig.set(ecs::getComponentId<Velocity>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& vel = mgr.getComponent<Velocity>(entity);

            // Lire inputs
            if (m_inputMgr->isKeyPressed(Key::Left)) {
                vel.vx = -200.0f;
            } else if (m_inputMgr->isKeyPressed(Key::Right)) {
                vel.vx = 200.0f;
            } else {
                vel.vx = 0.0f;
            }

            if (m_inputMgr->isKeyPressed(Key::Space)) {
                // Sauter
                vel.vy = -300.0f;
            }
        }
    }

private:
    InputManager* m_inputMgr;
};
```

---

## Patterns Avancés

### Pattern 1: Système avec Composant Optionnel

```cpp
class StatsSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        // Seulement besoin de Health comme requis
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Health>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& health = mgr.getComponent<Health>(entity);

            // Si Defence existe, l'utiliser
            if (mgr.hasComponent<Defence>(entity)) {
                auto& def = mgr.getComponent<Defence>(entity);
                // Réduire dégâts entrants selon défense
                health.hp = std::min(health.maxHp, health.hp + def.regenPerSec * deltaTime);
            }
        }
    }
};
```

### Pattern 2: Système avec Événements

```cpp
class DamageSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        return {};  // Pas de signature requise
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& eventBus = world.getEventBus();

        // S'abonner aux événements de collision
        eventBus.subscribe<CollisionEvent>(
            [&](const CollisionEvent& event) {
                handleCollision(world, event.e1, event.e2);
            }
        );
    }

private:
    void handleCollision(ecs::World& world, ecs::Entity e1, ecs::Entity e2) {
        auto& mgr = world.getComponentManager();

        // Si l'un a Damage et l'autre Health...
        if (mgr.hasComponent<Damage>(e1) && mgr.hasComponent<Health>(e2)) {
            int dmg = mgr.getComponent<Damage>(e1).amount;
            mgr.getComponent<Health>(e2).hp -= dmg;
        }
    }
};
```

### Pattern 3: Système Stateful

```cpp
class EffectsSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        return {};
    }

    void update(ecs::World& world, float deltaTime) override {
        // Particules, FX sound, screen shake, etc
        updateScreenShake(deltaTime);
        updateParticles(deltaTime);
    }

private:
    float m_shakeAmount = 0.0f;
    float m_shakeDuration = 0.0f;
    std::vector<Particle> m_particles;

    void updateScreenShake(float dt) {
        m_shakeDuration -= dt;
        if (m_shakeDuration < 0) m_shakeAmount = 0.0f;
    }

    void updateParticles(float dt) {
        m_particles.erase(
            std::remove_if(m_particles.begin(), m_particles.end(),
                [dt](Particle& p) {
                    p.lifetime -= dt;
                    return p.lifetime <= 0;
                }
            ),
            m_particles.end()
        );
    }
};
```

---

## Optimisation

### Optimisation 1: Utiliser getEntitiesWithSignature()

```cpp
// ✅ BON: O(1) avec bitset
auto entities = mgr.getEntitiesWithSignature(getSignature());
for (auto e : entities) { /* ... */ }

// ❌ MAUVAIS: O(n) itération complète
auto all_entities = mgr.getAllEntities();
for (auto e : all_entities) {
    if (mgr.hasComponent<A>(e) && mgr.hasComponent<B>(e)) { /* ... */ }
}
```

### Optimisation 2: Chunking pour Gros Systèmes

```cpp
class LargeSystem : public ecs::ISystem {
    const int CHUNK_SIZE = 256;

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Traiter par chunks pour meilleure localité cache
        for (size_t i = 0; i < entities.size(); i += CHUNK_SIZE) {
            size_t end = std::min(i + CHUNK_SIZE, entities.size());
            processChunk(world, i, end);
        }
    }
};
```

### Optimisation 3: Parallel Systems (C++17)

```cpp
#include <execution>

class ParallelSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Paralléliser sur plusieurs threads
        std::for_each(
            std::execution::par,
            entities.begin(),
            entities.end(),
            [&](ecs::Entity e) {
                auto& pos = mgr.getComponent<Position>(e);
                auto& vel = mgr.getComponent<Velocity>(e);
                pos.x += vel.vx * deltaTime;
                pos.y += vel.vy * deltaTime;
            }
        );
    }
};
```

---

## Débogage

### Affichage de Debug

```cpp
class DebugSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        return {};  // Affiche tout
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();

        std::cout << "=== DEBUG INFO ===" << std::endl;
        std::cout << "Entities alive: " << world.getEntityCount() << std::endl;

        // Afficher entités spécifiques
        auto posEntities = mgr.getEntitiesWithComponents<Position>();
        std::cout << "Entities with Position: " << posEntities.size() << std::endl;
    }
};
```

### Profiling

```cpp
class ProfilingSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto start = std::chrono::high_resolution_clock::now();

        // Code à profiler
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());
        for (auto e : entities) {
            // ...
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start
        );

        std::cout << "System took: " << duration.count() << " µs" << std::endl;
    }
};
```

---

## Summary

| Type | Utilisation |
|------|------------|
| **Physics System** | Mouvement, gravité, vélocité |
| **Collision System** | Détection et résolution collisions |
| **Combat System** | Dégâts, santé, morts |
| **Input System** | Lire contrôles joueur |
| **Render System** | Affichage graphique |
| **Spawn System** | Création entités dynamiques |
| **Effects System** | Animations, particules, FX |

Les systèmes sont la **logique du jeu**. Composez-les pour créer du gameplay complexe! 🎮
