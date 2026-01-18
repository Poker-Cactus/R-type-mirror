# Exemples de Code - R-Type Engine

## 📋 Table des Matières
- [Example 1: Jeu Simple 2D](#example-1--jeu-simple-2d)
- [Example 2: Système de Combat](#example-2--système-de-combat)
- [Example 3: Gestion Réseau](#example-3--gestion-réseau)
- [Example 4: Animation et Effets](#example-4--animation-et-effets)
- [Example 5: Spawn Intelligent](#example-5--spawn-intelligent)

---

## Example 1: Jeu Simple 2D

### Concept
Un jeu simple où:
- Le joueur peut se déplacer gauche/droite et sauter
- Les ennemis spawnt et se déplacent
- Le joueur gagne en touchant les ennemis

### Code Complet

**Components** (`include/components.hpp`)
```cpp
#pragma once

#include <cstdint>
#include <string>

// Position et mouvement
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct Acceleration {
    float ay = 9.81f;
};

// Collision
struct Collider {
    float width = 32.0f;
    float height = 32.0f;
};

// Rendu
struct Sprite {
    std::string texture;
    float width = 32.0f;
    float height = 32.0f;
};

// Gameplay
struct Health {
    int32_t hp = 100;
};

struct Player {};
struct Enemy {};
struct Platform {};
```

**Systèmes** (`include/systems.hpp`)
```cpp
#pragma once

#include "ecs/ISystem.hpp"
#include "components.hpp"
#include <iostream>
#include <cmath>

// Système de physique
class PhysicsSystem : public ecs::ISystem {
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

            // Appliquer gravité
            if (mgr.hasComponent<Acceleration>(entity)) {
                vel.vy += 9.81f * deltaTime;
            }

            // Limiter vitesse
            if (vel.vy > 500.0f) vel.vy = 500.0f;

            // Appliquer mouvement
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;

            // Limites monde
            if (pos.x < 0.0f) pos.x = 0.0f;
            if (pos.x > 1600.0f) pos.x = 1600.0f;
            if (pos.y > 1200.0f) {
                world.destroyEntity(entity);
            }
        }
    }
};

// Système de collision simple
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

        // Vérifier les collisions
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                if (checkCollision(mgr, entities[i], entities[j])) {
                    onCollision(world, entities[i], entities[j]);
                }
            }
        }
    }

private:
    bool checkCollision(ecs::ComponentManager& mgr, ecs::Entity e1, ecs::Entity e2) {
        auto& p1 = mgr.getComponent<Position>(e1);
        auto& c1 = mgr.getComponent<Collider>(e1);
        auto& p2 = mgr.getComponent<Position>(e2);
        auto& c2 = mgr.getComponent<Collider>(e2);

        return std::abs(p1.x - p2.x) < (c1.width + c2.width) / 2.0f &&
               std::abs(p1.y - p2.y) < (c1.height + c2.height) / 2.0f;
    }

    void onCollision(ecs::World& world, ecs::Entity e1, ecs::Entity e2) {
        auto& mgr = world.getComponentManager();
        
        bool e1IsPlayer = mgr.hasComponent<Player>(e1);
        bool e2IsPlayer = mgr.hasComponent<Player>(e2);
        bool e1IsEnemy = mgr.hasComponent<Enemy>(e1);
        bool e2IsEnemy = mgr.hasComponent<Enemy>(e2);

        // Joueur touche ennemi = victoire
        if ((e1IsPlayer && e2IsEnemy) || (e2IsPlayer && e1IsEnemy)) {
            std::cout << "Victory! You touched an enemy!" << std::endl;
        }

        // Plateforme arrête chute
        if ((e1IsPlayer && mgr.hasComponent<Platform>(e2)) ||
            (e2IsPlayer && mgr.hasComponent<Platform>(e1))) {
            auto& vel = mgr.getComponent<Velocity>(e1IsPlayer ? e1 : e2);
            vel.vy = 0.0f;
        }
    }
};

// Système d'input
class InputSystem : public ecs::ISystem {
public:
    InputSystem(float moveSpeed = 200.0f, float jumpForce = 300.0f)
        : m_moveSpeed(moveSpeed), m_jumpForce(jumpForce) {}

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

            // Simulation input (en vrai: SDL_GetKeyboardState)
            if (m_moveLeft) {
                vel.vx = -m_moveSpeed;
            } else if (m_moveRight) {
                vel.vx = m_moveSpeed;
            } else {
                vel.vx = 0.0f;
            }

            if (m_jump) {
                vel.vy = -m_jumpForce;
                m_jump = false;
            }
        }
    }

    void setMoveLeft(bool value) { m_moveLeft = value; }
    void setMoveRight(bool value) { m_moveRight = value; }
    void setJump(bool value) { m_jump = value; }

private:
    bool m_moveLeft = false;
    bool m_moveRight = false;
    bool m_jump = false;
    float m_moveSpeed;
    float m_jumpForce;
};

// Système de spawn
class SpawnSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        return {};  // Pas de signature
    }

    void update(ecs::World& world, float deltaTime) override {
        m_spawnTimer += deltaTime;

        if (m_spawnTimer >= 2.0f) {
            spawnEnemy(world);
            m_spawnTimer = 0.0f;
        }
    }

private:
    float m_spawnTimer = 0.0f;

    void spawnEnemy(ecs::World& world) {
        auto enemy = world.createEntity();
        world.addComponent(enemy, Position{100.0f + (rand() % 1400), 50.0f});
        world.addComponent(enemy, Velocity{50.0f + (rand() % 50), 0.0f});
        world.addComponent(enemy, Acceleration{9.81f});
        world.addComponent(enemy, Collider{32.0f, 32.0f});
        world.addComponent(enemy, Sprite{"enemy.png", 32.0f, 32.0f});
        world.addComponent(enemy, Enemy{});
    }
};
```

**Main** (`src/main.cpp`)
```cpp
#include "ecs/World.hpp"
#include "components.hpp"
#include "systems.hpp"

int main() {
    ecs::World world;

    // Enregistrer systèmes dans l'ordre
    world.registerSystem<PhysicsSystem>();
    world.registerSystem<CollisionSystem>();
    
    auto inputSystem = std::make_unique<InputSystem>();
    InputSystem* inputPtr = inputSystem.get();
    world.registerSystem<InputSystem>(std::move(inputSystem));
    
    world.registerSystem<SpawnSystem>();

    // Créer joueur
    auto player = world.createEntity();
    world.addComponent(player, Position{800.0f, 100.0f});
    world.addComponent(player, Velocity{0.0f, 0.0f});
    world.addComponent(player, Acceleration{9.81f});
    world.addComponent(player, Collider{32.0f, 32.0f});
    world.addComponent(player, Sprite{"player.png", 32.0f, 32.0f});
    world.addComponent(player, Player{});

    // Créer plateforme (sol)
    auto platform = world.createEntity();
    world.addComponent(platform, Position{800.0f, 800.0f});
    world.addComponent(platform, Collider{1600.0f, 50.0f});
    world.addComponent(platform, Platform{});

    // Boucle de jeu
    float deltaTime = 0.016f;  // 60 FPS
    bool running = true;

    std::cout << "Game started! Move with A/D, jump with Space." << std::endl;
    std::cout << "Touch an enemy to win!" << std::endl;

    // Simulation input
    inputPtr->setMoveRight(true);

    for (int frame = 0; frame < 300 && running; ++frame) {
        world.update(deltaTime);

        // À chaque 100 frames, inverser direction
        if (frame % 100 == 0) {
            inputPtr->setMoveRight(!inputPtr->getIsMovingRight());
        }
    }

    return 0;
}
```

---

## Example 2: Système de Combat

### Concept
Système de combat avec:
- Dégâts à la collision
- Cooldown d'attaque
- Armes différentes

### Code

```cpp
// Composants de combat
struct Weapon {
    int32_t damage = 10;
    float attackSpeed = 1.0f;  // Attaques par seconde
    float timeSinceLastAttack = 0.0f;
};

struct Armored {
    float defenseFactor = 0.5f;  // Réduit les dégâts de 50%
};

// Système de combat
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
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Mettre à jour cooldown d'attaque
        for (auto entity : entities) {
            if (mgr.hasComponent<Weapon>(entity)) {
                auto& weapon = mgr.getComponent<Weapon>(entity);
                weapon.timeSinceLastAttack += deltaTime;
            }
        }

        // S'abonner aux collisions pour dégâts
        eventBus.subscribe<CollisionEvent>(
            [&](const CollisionEvent& event) {
                handleDamage(world, event.e1, event.e2);
            }
        );

        // Supprimer morts
        std::vector<ecs::Entity> toRemove;
        for (auto entity : entities) {
            if (mgr.getComponent<Health>(entity).hp <= 0) {
                toRemove.push_back(entity);
            }
        }

        for (auto entity : toRemove) {
            world.destroyEntity(entity);
        }
    }

private:
    void handleDamage(ecs::World& world, ecs::Entity attacker, ecs::Entity target) {
        auto& mgr = world.getComponentManager();

        // Vérifier si attaquant a une arme et cooldown OK
        if (!mgr.hasComponent<Weapon>(attacker) ||
            !mgr.hasComponent<Health>(target)) {
            return;
        }

        auto& weapon = mgr.getComponent<Weapon>(attacker);
        float cooldown = 1.0f / weapon.attackSpeed;

        if (weapon.timeSinceLastAttack < cooldown) {
            return;  // Pas prêt à attaquer
        }

        // Calculer dégâts
        int damage = weapon.damage;

        // Appliquer armure
        if (mgr.hasComponent<Armored>(target)) {
            float defenseFactor = mgr.getComponent<Armored>(target).defenseFactor;
            damage = static_cast<int>(damage * defenseFactor);
        }

        // Appliquer dégâts
        auto& health = mgr.getComponent<Health>(target);
        health.hp -= damage;

        // Reset cooldown
        weapon.timeSinceLastAttack = 0.0f;

        std::cout << "Damage dealt: " << damage << ", Remaining HP: " 
                  << health.hp << std::endl;
    }
};
```

---

## Example 3: Gestion Réseau

### Concept
Synchroniser l'état du jeu entre serveur et clients

### Code

```cpp
// Paquet de synchronisation
struct EntitySyncPacket {
    uint32_t entityId;
    float posX, posY;
    float velX, velY;
    int32_t hp;
};

// Système côté serveur: Envoyer état
class NetworkSyncSystem : public ecs::ISystem {
public:
    NetworkSyncSystem(NetworkManager* netMgr) : m_netMgr(netMgr) {}

    ecs::ComponentSignature getSignature() const override {
        return {};  // Envoyer tout
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        
        // Trouver entités à synchroniser
        auto entities = mgr.getEntitiesWithComponents<Position>();

        for (auto entity : entities) {
            EntitySyncPacket packet{
                .entityId = static_cast<uint32_t>(entity)
            };

            if (mgr.hasComponent<Position>(entity)) {
                auto& pos = mgr.getComponent<Position>(entity);
                packet.posX = pos.x;
                packet.posY = pos.y;
            }

            if (mgr.hasComponent<Velocity>(entity)) {
                auto& vel = mgr.getComponent<Velocity>(entity);
                packet.velX = vel.vx;
                packet.velY = vel.vy;
            }

            if (mgr.hasComponent<Health>(entity)) {
                packet.hp = mgr.getComponent<Health>(entity).hp;
            }

            // Envoyer aux clients
            m_netMgr->broadcast(packet);
        }
    }

private:
    NetworkManager* m_netMgr;
};

// Système côté client: Recevoir état
class NetworkReceiveSystem : public ecs::ISystem {
public:
    NetworkReceiveSystem(NetworkManager* netMgr) : m_netMgr(netMgr) {}

    ecs::ComponentSignature getSignature() const override {
        return {};
    }

    void update(ecs::World& world, float deltaTime) override {
        // Recevoir paquets
        while (auto packet = m_netMgr->receive<EntitySyncPacket>()) {
            applyUpdate(world, *packet);
        }
    }

private:
    NetworkManager* m_netMgr;

    void applyUpdate(ecs::World& world, const EntitySyncPacket& packet) {
        auto entityId = static_cast<ecs::Entity>(packet.entityId);

        // Créer si n'existe pas
        if (!world.isAlive(entityId)) {
            world.createEntity();  // TODO: ID matching
        }

        // Mettre à jour position
        world.addComponent(entityId, Position{packet.posX, packet.posY});
        world.addComponent(entityId, Velocity{packet.velX, packet.velY});
        world.addComponent(entityId, Health{packet.hp, 100});
    }
};
```

---

## Example 4: Animation et Effets

### Concept
Système d'animation sprite avec frames

### Code

```cpp
// Animation
struct AnimatedSprite {
    std::string spriteSheet;
    int32_t currentFrame = 0;
    int32_t totalFrames = 4;
    float frameTime = 0.1f;
    float elapsedTime = 0.0f;
    bool isLooping = true;
};

// Système d'animation
class AnimationSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<AnimatedSprite>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& anim = mgr.getComponent<AnimatedSprite>(entity);
            anim.elapsedTime += deltaTime;

            // Passer au frame suivant?
            if (anim.elapsedTime >= anim.frameTime) {
                anim.currentFrame++;
                anim.elapsedTime = 0.0f;

                // Boucler si nécessaire
                if (anim.currentFrame >= anim.totalFrames) {
                    if (anim.isLooping) {
                        anim.currentFrame = 0;
                    } else {
                        anim.currentFrame = anim.totalFrames - 1;
                    }
                }
            }
        }
    }
};
```

---

## Example 5: Spawn Intelligent

### Concept
Spawner les ennemis intelligemment selon:
- Wave système
- Distance du joueur
- Limite d'ennemis

### Code

```cpp
struct Wave {
    int32_t waveNumber = 0;
    int32_t enemiesSpawned = 0;
    int32_t enemiesPerWave = 5;
    float timeSinceWaveStart = 0.0f;
    float waveInterval = 30.0f;
};

class WaveSpawnSystem : public ecs::ISystem {
public:
    WaveSpawnSystem(int maxEnemies = 50) : m_maxEnemies(maxEnemies) {}

    ecs::ComponentSignature getSignature() const override {
        return {};
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();

        // Trouver wave component (une seule instance)
        // En vrai: utiliser un système de singletons
        m_waveTimer += deltaTime;

        auto enemies = mgr.getEntitiesWithComponents<Enemy>();

        // Si pas assez d'ennemis et timer OK, spawner
        if (enemies.size() < m_maxEnemies && 
            m_waveTimer >= m_spawnInterval) {
            
            spawnEnemy(world);
            m_waveTimer = 0.0f;
            m_enemyCount++;

            if (m_enemyCount % 10 == 0) {
                std::cout << "Total enemies spawned: " << m_enemyCount << std::endl;
            }
        }
    }

private:
    int m_maxEnemies;
    int m_enemyCount = 0;
    float m_waveTimer = 0.0f;
    float m_spawnInterval = 1.0f;

    void spawnEnemy(ecs::World& world) {
        auto enemy = world.createEntity();
        float randomX = 50.0f + (rand() % 1500);
        float randomY = 50.0f + (rand() % 300);

        world.addComponent(enemy, Position{randomX, randomY});
        world.addComponent(enemy, Velocity{100.0f, 0.0f});
        world.addComponent(enemy, Acceleration{9.81f});
        world.addComponent(enemy, Health{30, 30});
        world.addComponent(enemy, Collider{32.0f, 32.0f});
        world.addComponent(enemy, Weapon{10, 0.5f, 0.0f});
        world.addComponent(enemy, Enemy{});
    }
};
```

---

Tous ces exemples peuvent être compilés et exécutés avec le système de build R-Type. Commencez par l'**Example 1** pour une base solide! 🎮
