# Composants et Signatures - Guide Technique

## 📋 Table des Matières
- [Définir les Composants](#définir-les-composants)
- [Component Signatures](#component-signatures)
- [Gestion des Composants](#gestion-des-composants)
- [Patterns Avancés](#patterns-avancés)
- [Sérialisation](#sérialisation)
- [Bonnes Pratiques](#bonnes-pratiques)

---

## Définir les Composants

### Qu'est-ce qu'un Composant?

Un composant est une **structure de données pure** qui représente une propriété:

```cpp
// ✅ BON: Données uniquement
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

// ❌ MAUVAIS: Mélanger données et logique
struct BadComponent {
    float x;
    void update(float dt) { x += 1.0f * dt; }  // ❌ Pas ici!
};
```

### Organisation

Créer une hiérarchie d'includes:

```cpp
// include/components/Core.hpp
// Composants utilisés partout (client + serveur)

#pragma once

#include <cstdint>
#include <string>

// ========== POSITIONAL ==========
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Rotation {
    float angle = 0.0f;  // En radians
};

// ========== MOVEMENT ==========
struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct Acceleration {
    float ax = 0.0f;
    float ay = 9.81f;
};

// ========== HEALTH & COMBAT ==========
struct Health {
    int32_t hp = 100;
    int32_t maxHp = 100;
};

struct Damage {
    int32_t amount = 10;
    float cooldown = 0.0f;
    float cooldownMax = 1.0f;
};

struct Defence {
    float armor = 0.0f;
    float blockChance = 0.0f;
};

// ========== PHYSICS ==========
struct Collider {
    float width = 32.0f;
    float height = 32.0f;
    bool isSolid = true;
};

struct Rigidbody {
    float mass = 1.0f;
    float friction = 0.0f;
    bool useGravity = true;
};

// ========== GRAPHICS ==========
struct Sprite {
    std::string texturePath;
    float width = 32.0f;
    float height = 32.0f;
    float opacity = 1.0f;
};

struct AnimatedSprite {
    std::string spriteSheet;
    int32_t currentFrame = 0;
    int32_t totalFrames = 1;
    float frameTime = 0.1f;
    float elapsedTime = 0.0f;
};

// ========== TAGS (Composants Vides) ==========
struct Player {};
struct Enemy {};
struct Projectile {};
struct PowerUp {};
struct Platform {};
struct Boss {};
```

### Séparation Client/Serveur

```cpp
// include/server/components/ServerOnly.hpp
// Composants **UNIQUEMENT** côté serveur

#pragma once

struct ServerState {
    uint64_t ownerId = 0;
    bool isMoving = false;
};

struct SpawnConfig {
    float spawnRate = 2.0f;
    int32_t maxCount = 10;
};

// include/client/components/ClientOnly.hpp
// Composants **UNIQUEMENT** côté client

#pragma once

struct LocalInput {
    float moveX = 0.0f;
    float moveY = 0.0f;
    bool jump = false;
};

struct AnimationState {
    bool isAnimating = false;
    float speed = 1.0f;
};
```

---

## Component Signatures

### Qu'est-ce qu'une Signature?

Une signature est un **bitset** qui indique quels composants une entité doit avoir:

```cpp
// Interne: std::bitset<MAX_COMPONENTS>
using ComponentSignature = std::bitset<256>;

ComponentSignature sig;
sig.set(getComponentId<Position>());   // Bit pour Position = 1
sig.set(getComponentId<Velocity>());   // Bit pour Velocity = 1
// Tous les autres bits = 0

// Format binaire (exemple simplifié)
// sig = 0011000000...0000
```

### Utiliser les Signatures

```cpp
class MovementSystem : public ecs::ISystem {
public:
    // Déclarer la signature requise
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Velocity>());
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();

        // Récupérer UNIQUEMENT les entités avec Position + Velocity
        // Très efficace! O(1) avec bitwise AND
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& pos = mgr.getComponent<Position>(entity);
            auto& vel = mgr.getComponent<Velocity>(entity);
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }
};
```

### IDs des Composants

Chaque composant reçoit un ID unique:

```cpp
// Automatiquement attribué à la première utilisation
ecs::getComponentId<Position>();   // ID = 0
ecs::getComponentId<Velocity>();   // ID = 1
ecs::getComponentId<Health>();     // ID = 2
// ...

// Les IDs restent constants pendant l'exécution
```

---

## Gestion des Composants

### Ajouter des Composants

```cpp
auto entity = world.createEntity();

// Ajouter avec valeurs par défaut
world.addComponent(entity, Position{});  // x=0, y=0

// Ajouter avec valeurs spécifiques
world.addComponent(entity, Position{100.0f, 50.0f});
world.addComponent(entity, Velocity{10.0f, 0.0f});
world.addComponent(entity, Health{100, 100});
world.addComponent(entity, Damage{25, 0.0f, 1.0f});
```

### Accéder aux Composants

```cpp
auto& mgr = world.getComponentManager();

// Accès direct (non-const)
auto& pos = mgr.getComponent<Position>(entity);
pos.x += 10.0f;

// Accès const
const auto& vel = mgr.getComponent<Velocity>(entity);
float speed = std::sqrt(vel.vx * vel.vx + vel.vy * vel.vy);
```

### Vérifier la Présence

```cpp
auto& mgr = world.getComponentManager();

if (mgr.hasComponent<Health>(entity)) {
    auto& health = mgr.getComponent<Health>(entity);
    health.hp -= 10;
}
```

### Modifier des Composants

```cpp
auto& mgr = world.getComponentManager();

// Remplacer complètement
mgr.addComponent(entity, Position{200.0f, 100.0f});  // Remplace si existe

// Modifier partiellement
auto& pos = mgr.getComponent<Position>(entity);
pos.x = 200.0f;
pos.y = 100.0f;
```

### Supprimer des Composants

```cpp
auto& mgr = world.getComponentManager();

// Supprimer un composant spécifique
mgr.removeComponent<Animation>(entity);

// Supprimer tous les composants d'une entité
mgr.removeAllComponents(entity);
```

---

## Patterns Avancés

### Pattern 1: Composants Hiérarchiques

```cpp
// Composant de base
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

// Composants spécialisés
struct PhysicalBody {
    float mass = 1.0f;
    float friction = 0.0f;
    float restitution = 0.0f;
};

struct DynamicBody : PhysicalBody {
    // Ajouter comportement dynamique
};

// Utilisation
auto entity = world.createEntity();
world.addComponent(entity, Transform{});
world.addComponent(entity, DynamicBody{});
```

### Pattern 2: Composants de Configuration

```cpp
// Composants de config (définis une fois)
struct ProjectileConfig {
    float speed = 100.0f;
    float lifetime = 5.0f;
    int32_t damage = 10;
};

struct EnemyConfig {
    std::string type;
    float health = 50.0f;
    float speed = 50.0f;
    int32_t reward = 100;
};

// Utilisation
auto projectile = world.createEntity();
world.addComponent(projectile, ProjectileConfig{
    .speed = 200.0f,
    .lifetime = 10.0f,
    .damage = 50
});
```

### Pattern 3: Composants d'État Transitoire

```cpp
// État temporaire (durée limitée)
struct BuffEffect {
    std::string name;
    float duration = 5.0f;
    float damageBonus = 1.5f;
    float speedBonus = 1.2f;
};

struct DamageOverTime {
    float damagePerSecond = 10.0f;
    float duration = 3.0f;
    std::string sourceId;
};

// Utilisation
auto entity = world.createEntity();
world.addComponent(entity, BuffEffect{
    .name = "Berserk",
    .duration = 5.0f,
    .damageBonus = 2.0f
});

// Dans un système:
auto& buf = mgr.getComponent<BuffEffect>(entity);
buf.duration -= deltaTime;
if (buf.duration <= 0.0f) {
    mgr.removeComponent<BuffEffect>(entity);
}
```

### Pattern 4: Composants de Liaison (References)

```cpp
// Important: Stocker des Entity IDs, PAS des pointeurs!
struct Projectile {
    ecs::Entity owner;      // ✅ BON: ID
    ecs::Entity target;     // ✅ BON: ID
};

struct Child {
    ecs::Entity parent;     // ✅ BON: ID (relation parent-enfant)
};

// ❌ MAUVAIS: Ne jamais stocker de pointeurs
// struct BadComponent {
//     Position* posPtr;     // ❌ INVALIDE!
//     Enemy* enemyPtr;      // ❌ Peut être supprimé!
// };
```

---

## Sérialisation

### Préparation pour Réseau

Les composants doivent être **sérialisables** pour le réseau:

```cpp
// include/components/Serializable.hpp

#pragma once

struct Position {
    float x = 0.0f;
    float y = 0.0f;

    // Sérialisation
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(8);
        std::memcpy(data.data(), &x, sizeof(float));
        std::memcpy(data.data() + 4, &y, sizeof(float));
        return data;
    }

    // Désérialisation
    static Position deserialize(const std::vector<uint8_t>& data) {
        Position pos;
        std::memcpy(&pos.x, data.data(), sizeof(float));
        std::memcpy(&pos.y, data.data() + 4, sizeof(float));
        return pos;
    }
};

struct Health {
    int32_t hp = 100;
    int32_t maxHp = 100;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(8);
        std::memcpy(data.data(), &hp, sizeof(int32_t));
        std::memcpy(data.data() + 4, &maxHp, sizeof(int32_t));
        return data;
    }

    static Health deserialize(const std::vector<uint8_t>& data) {
        Health h;
        std::memcpy(&h.hp, data.data(), sizeof(int32_t));
        std::memcpy(&h.maxHp, data.data() + 4, sizeof(int32_t));
        return h;
    }
};
```

### Exemple de Synchronisation

```cpp
// Serveur -> Clients: Envoyer état
std::vector<uint8_t> serializeEntity(const ecs::World& world, ecs::Entity entity) {
    auto& mgr = world.getComponentManager();
    std::vector<uint8_t> buffer;

    // Ajouter Position
    if (mgr.hasComponent<Position>(entity)) {
        auto data = mgr.getComponent<Position>(entity).serialize();
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    // Ajouter Health
    if (mgr.hasComponent<Health>(entity)) {
        auto data = mgr.getComponent<Health>(entity).serialize();
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    return buffer;
}

// Clients: Récupérer état
void deserializeEntity(ecs::World& world, ecs::Entity entity,
                       const std::vector<uint8_t>& buffer) {
    world.addComponent(entity, Position::deserialize(
        std::vector<uint8_t>(buffer.begin(), buffer.begin() + 8)
    ));
    
    world.addComponent(entity, Health::deserialize(
        std::vector<uint8_t>(buffer.begin() + 8, buffer.begin() + 16)
    ));
}
```

---

## Bonnes Pratiques

### ✅ À Faire

1. **Une responsabilité par composant**
   ```cpp
   struct Position { float x, y; };      // Juste position
   struct Velocity { float vx, vy; };    // Juste vitesse
   struct Health { int hp; };            // Juste santé
   ```

2. **Composants légers**
   ```cpp
   struct Health { int32_t hp, maxHp; };  // 8 bytes

   // Pas:
   struct BadHealth {
       int32_t hp, maxHp;
       std::vector<Debuff> debuffs;       // Trop lourd!
       std::string deathReason;
   };
   ```

3. **Utiliser les types appropriés**
   ```cpp
   struct Position {
       float x, y;              // Position: float
   };

   struct Health {
       int32_t hp, maxHp;       // Santé: entier
   };

   struct PlayerId {
       uint64_t id;             // ID: uint64
   };
   ```

4. **Composants sérialisables**
   ```cpp
   // BON: Simple à sérialiser
   struct Position { float x, y; };

   // MAUVAIS: Pas facile à sérialiser
   struct BadComponent {
       std::unique_ptr<Data> ptr;
       std::vector<std::string> names;
   };
   ```

### ❌ À Éviter

1. **Logique dans les composants**
   ```cpp
   // ❌ NON
   struct BadHealth {
       int hp;
       void takeDamage(int dmg) { hp -= dmg; }
   };
   ```

2. **Dépendances entre composants**
   ```cpp
   // ❌ NON: Position ne doit pas dépendre de Transform
   struct Position {
       Transform* parent;
   };
   ```

3. **Composants trop gros**
   ```cpp
   // ❌ NON: Trop de données inutiles
   struct Entity {
       float x, y, vx, vy, ax, ay;
       int hp, maxHp, mana, maxMana;
       std::string name, type, faction;
       // ... 50 autres champs
   };
   ```

4. **Cacher les données**
   ```cpp
   // ❌ NON: Les composants doivent être du pure data
   struct HiddenComponent {
   private:
       int value;
       int getValue() { return value; }
   };
   ```

---

## Summary

| Concept | Utilité |
|---------|---------|
| **Component** | Structure de données pure |
| **Signature** | Filtre pour systèmes (bitset) |
| **Tag** | Composant vide pour marquage |
| **Config** | Composant de configuration |
| **State** | Composant d'état transitoire |

Les composants sont la **donnée du jeu**. Conçois-les simples, sérialisables et réutilisables! 🎮
