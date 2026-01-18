# ECS Engine - Architecture Complète

## 📋 Table des Matières
- [Introduction](#introduction)
- [Concepts Fondamentaux](#concepts-fondamentaux)
- [Architecture Détaillée](#architecture-détaillée)
- [Workflow Typique](#workflow-typique)
- [Exemples Pratiques](#exemples-pratiques)
- [Bonnes Pratiques](#bonnes-pratiques)

---

## Introduction

Le système **Entity-Component-System (ECS)** est un pattern architectural qui sépare les données (composants) de la logique (systèmes). C'est l'architecture de base du moteur R-Type.

### Avantages de l'ECS

- ✅ **Flexibilité** : Combinez facilement des composants pour créer des entités complexes
- ✅ **Performance** : Données en mémoire contiguë, cache-friendly
- ✅ **Réutilisabilité** : Composants et systèmes peuvent être réutilisés dans différents jeux
- ✅ **Testabilité** : Logique décorrélée des données
- ✅ **Scalabilité** : Facile d'ajouter de nouvelles fonctionnalités

---

## Concepts Fondamentaux

### 1. **Entity (Entité)**

Une entité est un conteneur identifier unique pour regrouper des composants. C'est juste un ID:

```cpp
using Entity = std::uint32_t;
```

```cpp
// Créer une entité
Entity player = world.createEntity();  // ID unique (ex: 1, 2, 3...)
Entity enemy = world.createEntity();
```

**Caractéristiques:**
- ID unique et immuable
- Pas de comportement propre
- Sert de clé pour accéder aux composants associés

### 2. **Component (Composant)**

Un composant est une structure de données pure qui représente une propriété ou un état:

```cpp
// Position dans l'espace
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

// Vitesse et direction
struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

// Points de vie
struct Health {
    int hp = 100;
    int maxHp = 100;
};

// Simple marqueur (présence/absence)
struct Player {};
struct Enemy {};
```

**Principes:**
- Pur data (pas de méthodes)
- Sérialisable
- Léger (quelques bytes)
- Réutilisable dans d'autres jeux

```cpp
// Ajouter des composants à une entité
world.addComponent(player, Position{100.0f, 50.0f});
world.addComponent(player, Velocity{10.0f, 0.0f});
world.addComponent(player, Health{100, 100});
world.addComponent(player, Player{});  // Marqueur

world.addComponent(enemy, Position{500.0f, 50.0f});
world.addComponent(enemy, Velocity{-5.0f, 0.0f});
world.addComponent(enemy, Health{30, 30});
world.addComponent(enemy, Enemy{});  // Marqueur
```

### 3. **System (Système)**

Un système contient la logique qui opère sur les entités ayant certains composants:

```cpp
class MovementSystem : public ISystem {
public:
    /**
     * Définir la signature: quels composants ce système nécessite
     * Les entities sans ces composants seront ignorées
     */
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Position>());
        sig.set(ecs::getComponentId<Velocity>());
        return sig;
    }

    /**
     * Logique du système exécutée chaque frame
     */
    void update(ecs::World& world, float deltaTime) override {
        auto& componentMgr = world.getComponentManager();
        auto entities = componentMgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& pos = componentMgr.getComponent<Position>(entity);
            auto& vel = componentMgr.getComponent<Velocity>(entity);

            // Mettre à jour la position basée sur la vélocité
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }
};
```

### 4. **World (Monde)**

Le `World` est le coordinateur central qui gère tout:

```cpp
ecs::World world;

// Crée entités
Entity player = world.createEntity();

// Gère composants
world.addComponent(player, Position{0.0f, 0.0f});
world.removeComponent<Velocity>(player);

// Enregistre systèmes
world.registerSystem<MovementSystem>();
world.registerSystem<RenderSystem>();

// Boucle de jeu
float deltaTime = 0.016f;  // 60 FPS
while (running) {
    world.update(deltaTime);  // Exécute tous les systèmes
}
```

---

## Architecture Détaillée

### Hiérarchie des Composants

```
IComponentStorage (Interface)
    ↓
ComponentStorage<T> (Template spécialisé)
```

Chaque type de composant `T` a son propre stockage optimisé:

```cpp
struct Position { float x, y; };

// Stockage interne pour Position
ComponentStorage<Position> storage;
storage.add(entity_1, Position{10.0f, 20.0f});
storage.add(entity_2, Position{30.0f, 40.0f});

// Accès rapide O(1)
Position& pos = storage.get(entity_1);  // Position{10.0f, 20.0f}
```

### Component Signature

La signature permet de filtrer rapidement les entités:

```cpp
// Bitset qui indique quels composants sont requis
using ComponentSignature = std::bitset<MAX_COMPONENTS>;

class AttackSystem : public ISystem {
    ComponentSignature getSignature() const override {
        ComponentSignature sig;
        sig.set(getComponentId<Position>());      // Besoin position
        sig.set(getComponentId<Damage>());        // Besoin dégâts
        sig.set(getComponentId<AttackCooldown>()); // Besoin cooldown
        return sig;
    }
};

// Seules les entités ayant Position + Damage + AttackCooldown
// seront traitées par ce système O(1) check avec bitwise AND
```

### Managers

```
World
├── EntityManager      (crée/détruit entités)
├── ComponentManager   (gère l'ajout/retrait de composants)
├── SystemManager      (enregistre/exécute systèmes)
└── EventBus          (système d'événements)
```

---

## Workflow Typique

### Étape 1: Initialisation

```cpp
ecs::World world;

// Enregistrer tous les systèmes
world.registerSystem<SpawnSystem>();
world.registerSystem<InputSystem>();
world.registerSystem<MovementSystem>();
world.registerSystem<CollisionSystem>();
world.registerSystem<HealthSystem>();
world.registerSystem<RenderSystem>();
```

### Étape 2: Création d'Entités

```cpp
Entity player = world.createEntity();
world.addComponent(player, Position{800.0f, 600.0f});
world.addComponent(player, Velocity{0.0f, 0.0f});
world.addComponent(player, Health{100, 100});
world.addComponent(player, Player{});
world.addComponent(player, Sprite{"player.png"});

Entity enemy = world.createEntity();
world.addComponent(enemy, Position{100.0f, 100.0f});
world.addComponent(enemy, Velocity{-50.0f, 0.0f});
world.addComponent(enemy, Health{30, 30});
world.addComponent(enemy, Enemy{});
world.addComponent(enemy, Sprite{"enemy.png"});
```

### Étape 3: Boucle de Jeu

```cpp
while (game.isRunning()) {
    float deltaTime = game.getFrameTime();
    
    // Tous les systèmes s'exécutent dans l'ordre enregistré
    world.update(deltaTime);
    
    // Order of execution:
    // 1. SpawnSystem   - crée nouvelles entités
    // 2. InputSystem   - lit les inputs joueur
    // 3. MovementSystem - met à jour les positions
    // 4. CollisionSystem - détecte collisions
    // 5. HealthSystem   - gère dégâts et morts
    // 6. RenderSystem   - affiche les sprites
}
```

### Étape 4: Destruction

```cpp
world.destroyEntity(enemy);  // Supprime tous les composants
```

---

## Exemples Pratiques

### Exemple 1: Système Simple de Mouvement

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

            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }
};
```

### Exemple 2: Système avec Composant Optionnel

```cpp
class DamageSystem : public ecs::ISystem {
public:
    ecs::ComponentSignature getSignature() const override {
        ecs::ComponentSignature sig;
        sig.set(ecs::getComponentId<Health>());
        // Pas de vérification de Poison - composant optionnel
        return sig;
    }

    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        for (auto entity : entities) {
            auto& health = mgr.getComponent<Health>(entity);

            // Check optionnel pour Poison
            if (mgr.hasComponent<Poison>(entity)) {
                auto& poison = mgr.getComponent<Poison>(entity);
                health.hp -= poison.damagePerSecond * deltaTime;
                poison.duration -= deltaTime;

                if (poison.duration <= 0.0f) {
                    mgr.removeComponent<Poison>(entity);
                }
            }

            // Entité morte?
            if (health.hp <= 0) {
                world.destroyEntity(entity);
            }
        }
    }
};
```

### Exemple 3: Système avec Événements

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
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Détection de collisions brute-force O(n²)
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto e1 = entities[i];
                auto e2 = entities[j];

                if (checkCollision(mgr.getComponent<Position>(e1),
                                   mgr.getComponent<Collider>(e1),
                                   mgr.getComponent<Position>(e2),
                                   mgr.getComponent<Collider>(e2))) {
                    
                    // Émettre événement
                    world.getEventBus().emit<CollisionEvent>(e1, e2);
                }
            }
        }
    }

private:
    bool checkCollision(const Position& p1, const Collider& c1,
                        const Position& p2, const Collider& c2) {
        return std::abs(p1.x - p2.x) < (c1.width + c2.width) / 2.0f &&
               std::abs(p1.y - p2.y) < (c1.height + c2.height) / 2.0f;
    }
};
```

---

## Bonnes Pratiques

### ✅ À Faire

1. **Composants petits et spécialisés**
   ```cpp
   // BON: Une responsabilité par composant
   struct Position { float x, y; };
   struct Velocity { float vx, vy; };

   // MAUVAIS: Trop de données
   struct Transform {
       float x, y, vx, vy, rotation, scale;
       float mass, friction, elasticity;  // Non lié à transform
   };
   ```

2. **Systèmes indépendants et réutilisables**
   ```cpp
   // BON: Pas de dépendance sur les détails R-Type
   class MovementSystem : public ISystem {
       // Fonctionne dans n'importe quel jeu
   };
   ```

3. **Utiliser les signatures pour le filtering**
   ```cpp
   // BON: O(1) grâce au bitset
   auto entities = mgr.getEntitiesWithSignature(getSignature());

   // MAUVAIS: O(n) itération inefficace
   for (auto entity : all_entities) {
       if (mgr.hasComponent<A>(entity) && 
           mgr.hasComponent<B>(entity)) {
           // ...
       }
   }
   ```

4. **Ordre des systèmes important**
   ```cpp
   world.registerSystem<InputSystem>();        // 1. Lire inputs
   world.registerSystem<MovementSystem>();     // 2. Mettre à jour positions
   world.registerSystem<CollisionSystem>();    // 3. Vérifier collisions
   world.registerSystem<RenderSystem>();       // 4. Afficher
   ```

### ❌ À Éviter

1. **Composants avec logique**
   ```cpp
   // MAUVAIS
   struct Health {
       int hp;
       void takeDamage(int dmg) { hp -= dmg; }  // ❌ Logique!
   };
   ```

2. **Systèmes qui se modifient eux-mêmes**
   ```cpp
   class BadSystem {
       void update(World& w, float dt) {
           // ❌ Avoid modifying world during iteration!
           w.destroyEntity(entity);
       }
   };
   ```

3. **Trop de composants par entité**
   ```cpp
   // ❌ Les entités ne doivent avoir que les composants nécessaires
   world.addComponent(player, ComponentA{});
   world.addComponent(player, ComponentB{});
   world.addComponent(player, ComponentC{});
   // ... 20+ composants inutiles!
   ```

---

## Résumé

| Concept | Rôle | Exemple |
|---------|------|---------|
| **Entity** | Identifiant unique | `Entity player = 1` |
| **Component** | Données pures | `Position{10, 20}` |
| **System** | Logique métier | `MovementSystem` |
| **World** | Coordinateur | `world.update(dt)` |
| **Signature** | Filtre d'entités | `requires<Position, Velocity>` |

Le pattern ECS permet de construire des jeux complexes en composant simplement des briques réutilisables!
