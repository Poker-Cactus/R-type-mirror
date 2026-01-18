# Architecture & Best Practices - R-Type Engine

## 📋 Table des Matières
- [Architecture Globale](#architecture-globale)
- [Patterns de Conception](#patterns-de-conception)
- [Client vs Serveur](#client-vs-serveur)
- [Performance & Optimisation](#performance--optimisation)
- [Bonnes Pratiques](#bonnes-pratiques)
- [Checklist de Projet](#checklist-de-projet)

---

## Architecture Globale

### Vue d'Ensemble

```
┌─────────────────────────────────────────────────────┐
│                   R-Type Engine                     │
├─────────────────────────────────────────────────────┤
│  engineCore/                                        │
│  ├── ECS (Entity-Component-System)                  │
│  ├── Registry (World, EntityManager, etc.)         │
│  └── Utils (Logger, Timer, etc.)                   │
└─────────────────────────────────────────────────────┘
         ↑                              ↑
    ├────┴────────┬───────────────────┘
    │             │
┌───┴───┐    ┌────┴────┐
│ SERVER│    │ CLIENT   │
├───────┤    ├──────────┤
│Systems│    │Systems   │
│-Phys  │    │-Input    │
│-Coll  │    │-Render   │
│-Spawn │    │-FX       │
│-Combat│    │-Audio    │
└───────┘    └──────────┘
```

### Hiérarchie de Dépendances

```
engineCore (Lib)
    ↑
    ├── common (Lib)
    │   ├── components/
    │   ├── protocol/
    │   └── systems/
    │
    ├── server (Exe)
    │   ├── systems/
    │   └── network/
    │
    └── client (Exe)
        ├── systems/
        └── rendering/
```

### Points Clés

- ✅ **Aucune circulaire dépendance**
- ✅ **Common** est une couche de partage
- ✅ **engineCore** est agnostique du jeu
- ✅ **Server** et **Client** sont indépendants

---

## Patterns de Conception

### Pattern 1: Dependency Injection

```cpp
// ✅ BON: Injecter les dépendances
class RenderSystem : public ecs::ISystem {
public:
    RenderSystem(SDL_Renderer* renderer)
        : m_renderer(renderer) {}

private:
    SDL_Renderer* m_renderer;  // Dépendance injectée
};

// Utilisation
SDL_Renderer* renderer = initSDL();
world.registerSystem<RenderSystem>(new RenderSystem(renderer));

// ❌ MAUVAIS: Créer de nouveau ressources
class BadRenderSystem {
public:
    BadRenderSystem() {
        m_renderer = SDL_CreateRenderer(...);  // ❌ Couplage!
    }
};
```

### Pattern 2: Object Pool pour Performance

```cpp
// Réutiliser les entités au lieu de les créer/détruire
class ProjectilePool {
private:
    std::vector<ecs::Entity> m_pool;
    ecs::World& m_world;
    size_t m_poolSize;

public:
    ProjectilePool(ecs::World& w, size_t size)
        : m_world(w), m_poolSize(size) {
        for (size_t i = 0; i < size; ++i) {
            m_pool.push_back(m_world.createEntity());
        }
    }

    ecs::Entity acquire(float x, float y) {
        if (m_pool.empty()) {
            return m_world.createEntity();  // Créer si manque
        }

        auto entity = m_pool.back();
        m_pool.pop_back();

        // Réinitialiser l'entité
        m_world.addComponent(entity, Position{x, y});
        m_world.addComponent(entity, Velocity{100.0f, 0.0f});
        m_world.addComponent(entity, Projectile{});

        return entity;
    }

    void release(ecs::Entity entity) {
        if (m_world.isAlive(entity)) {
            m_world.destroyEntity(entity);
        }
        m_pool.push_back(entity);
    }
};

// Utilisation
ProjectilePool projectiles(world, 100);

// Créer projectile (réutilisé du pool)
auto proj = projectiles.acquire(pos.x, pos.y);

// Supprimer (retour au pool)
projectiles.release(proj);
```

### Pattern 3: Factory pour Création d'Entités

```cpp
class EntityFactory {
public:
    static ecs::Entity createPlayer(ecs::World& world, float x, float y) {
        auto entity = world.createEntity();
        world.addComponent(entity, Position{x, y});
        world.addComponent(entity, Velocity{0.0f, 0.0f});
        world.addComponent(entity, Health{100, 100});
        world.addComponent(entity, Collider{32.0f, 32.0f});
        world.addComponent(entity, Sprite{"player.png"});
        world.addComponent(entity, Player{});
        return entity;
    }

    static ecs::Entity createEnemy(ecs::World& world, float x, float y,
                                    const std::string& type) {
        auto entity = world.createEntity();
        world.addComponent(entity, Position{x, y});
        world.addComponent(entity, Health{30, 30});
        world.addComponent(entity, Collider{32.0f, 32.0f});
        world.addComponent(entity, Enemy{});
        
        if (type == "fast") {
            world.addComponent(entity, Velocity{200.0f, 0.0f});
        } else if (type == "strong") {
            world.addComponent(entity, Velocity{50.0f, 0.0f});
            world.getComponentManager()
                .getComponent<Health>(entity).maxHp = 100;
        }
        
        return entity;
    }

    static ecs::Entity createProjectile(ecs::World& world,
                                        ecs::Entity owner,
                                        float x, float y) {
        auto entity = world.createEntity();
        world.addComponent(entity, Position{x, y});
        world.addComponent(entity, Velocity{300.0f, 0.0f});
        world.addComponent(entity, Damage{25});
        world.addComponent(entity, Projectile{});
        // Lier au propriétaire
        world.addComponent(entity, OwnerData{owner});
        return entity;
    }
};

// Utilisation
auto player = EntityFactory::createPlayer(world, 400, 300);
auto enemy = EntityFactory::createEnemy(world, 100, 200, "fast");
auto proj = EntityFactory::createProjectile(world, player, 410, 300);
```

### Pattern 4: Event-Driven Architecture

```cpp
// Événements pour découpler systèmes
struct PlayerDamagedEvent {
    ecs::Entity player;
    int damage;
};

struct EnemyDefeatedEvent {
    ecs::Entity enemy;
    int reward;
};

// Système 1: Dégâts
class DamageSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& eventBus = world.getEventBus();
        
        // Émettre événement quand joueur pris dégâts
        eventBus.emit<PlayerDamagedEvent>(player, 10);
    }
};

// Système 2: Gestion santé
class HealthSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& eventBus = world.getEventBus();
        
        // Écouter événement dégâts
        eventBus.subscribe<PlayerDamagedEvent>(
            [&](const PlayerDamagedEvent& evt) {
                auto& health = world.getComponentManager()
                    .getComponent<Health>(evt.player);
                health.hp -= evt.damage;
            }
        );
    }
};

// Système 3: UI / Audio
class UISystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& eventBus = world.getEventBus();
        
        // S'abonner à plusieurs événements
        eventBus.subscribe<PlayerDamagedEvent>(
            [](const PlayerDamagedEvent& evt) {
                playSound("damage.wav");
                updateHealthBar(evt.player);
            }
        );

        eventBus.subscribe<EnemyDefeatedEvent>(
            [](const EnemyDefeatedEvent& evt) {
                playSound("victory.wav");
                updateScore(evt.reward);
            }
        );
    }
};
```

---

## Client vs Serveur

### Séparation des Responsabilités

#### Côté Serveur
```cpp
// Server: Authorité complète sur l'état du jeu

class ServerGameLoop {
    ecs::World world;  // État vérité unique

    void run() {
        world.registerSystem<InputHandlingSystem>();      // Recevoit inputs
        world.registerSystem<MovementSystem>();           // Applique mouvements
        world.registerSystem<PhysicsSystem>();            // Calcule physique
        world.registerSystem<CollisionSystem>();          // Détecte collisions
        world.registerSystem<CombatSystem>();             // Gère combat
        world.registerSystem<NetworkSyncSystem>();        // Envoie état clients

        while (running) {
            world.update(deltaTime);
            receiveClientInputs();
            syncWithClients();
        }
    }
};
```

#### Côté Client
```cpp
// Client: Affichage + Input + Prédiction optionnelle

class ClientGameLoop {
    ecs::World world;  // Réplique partielle du serveur

    void run() {
        world.registerSystem<InputSystem>();              // Lit inputs locaux
        world.registerSystem<PredictionSystem>();         // Prédiction côté client
        world.registerSystem<RenderSystem>();             // Affiche
        world.registerSystem<AudioSystem>();              // Sons
        world.registerSystem<NetworkReceiveSystem>();     // Reçoit état serveur

        while (running) {
            world.update(deltaTime);
            sendInputs();
            receiveServerUpdates();
            render();
        }
    }
};
```

### Synchronisation Réseau

```cpp
// État à synchroniser
struct NetworkState {
    ecs::Entity entity;
    Position position;
    Health health;
    Animation animation;
};

// Côté serveur: Envoyer
class NetworkSyncSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        // Envoyer position + santé de chaque entité
        auto players = world.getComponentManager()
            .getEntitiesWithComponents<Position, Health>();

        for (auto entity : players) {
            NetworkState state{
                entity,
                world.getComponentManager().getComponent<Position>(entity),
                world.getComponentManager().getComponent<Health>(entity)
            };
            
            sendToClients(serialize(state));
        }
    }
};

// Côté client: Recevoir
void onNetworkUpdate(const NetworkState& state) {
    if (!world.isAlive(state.entity)) {
        world.createEntity();  // Créer si nouveau
    }

    // Mettre à jour composants
    world.addComponent(state.entity, state.position);
    world.addComponent(state.entity, state.health);
    world.addComponent(state.entity, state.animation);
}
```

---

## Performance & Optimisation

### Optimisation 1: Spatial Partitioning

```cpp
// QuadTree pour détection collision O(n log n) au lieu O(n²)
class QuadTreeCollisionSystem : public ecs::ISystem {
private:
    struct QuadTree {
        std::vector<ecs::Entity> entities;
        QuadTree* children[4] = {nullptr};
        
        void insert(ecs::Entity entity, const Position& pos) {
            if (children[0] == nullptr) {
                entities.push_back(entity);
                if (entities.size() > MAX_PER_NODE) split();
            } else {
                int child = getChild(pos);
                children[child]->insert(entity, pos);
            }
        }
    };

    QuadTree tree;

public:
    void update(ecs::World& world, float deltaTime) override {
        // Utiliser quadtree pour détection
        tree.query(getViewportBounds());
    }
};
```

### Optimisation 2: System Ordering

```cpp
// Ordonner systèmes pour meilleure cache locality
world.registerSystem<PhysicsSystem>();        // Modifie Position
world.registerSystem<CollisionSystem>();      // Lit Position
world.registerSystem<AnimationSystem>();      // Modifie Sprite
world.registerSystem<RenderSystem>();         // Lit Sprite

// Bon ordre: grouper les systèmes accédant les mêmes composants
```

### Optimisation 3: Parallel Processing

```cpp
#include <execution>
#include <algorithm>

class ParallelMovementSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Paralléliser sur plusieurs threads
        std::for_each(
            std::execution::par,
            entities.begin(),
            entities.end(),
            [&, deltaTime](ecs::Entity e) {
                auto& pos = mgr.getComponent<Position>(e);
                auto& vel = mgr.getComponent<Velocity>(e);
                pos.x += vel.vx * deltaTime;
                pos.y += vel.vy * deltaTime;
            }
        );
    }
};
```

### Optimisation 4: Update Batching

```cpp
class BatchedRenderSystem : public ecs::ISystem {
    void update(ecs::World& world, float deltaTime) override {
        auto& mgr = world.getComponentManager();
        auto entities = mgr.getEntitiesWithSignature(getSignature());

        // Batching par texture
        std::map<std::string, std::vector<RenderData>> batches;

        for (auto entity : entities) {
            auto& sprite = mgr.getComponent<Sprite>(entity);
            auto& pos = mgr.getComponent<Position>(entity);
            
            RenderData data{pos.x, pos.y, sprite.width, sprite.height};
            batches[sprite.texturePath].push_back(data);
        }

        // Render batch by batch
        for (auto& [texture, renderDatas] : batches) {
            bindTexture(texture);
            drawBatch(renderDatas);  // Une seule draw call par texture
        }
    }
};
```

---

## Bonnes Pratiques

### ✅ À Faire

1. **Composants petits et légers**
   ```cpp
   struct Position { float x, y; };        // 8 bytes
   struct Velocity { float vx, vy; };      // 8 bytes
   // Pas 100+ bytes par composant
   ```

2. **Systèmes découplés**
   ```cpp
   // Systèmes ne connaissent pas l'existence des autres
   class PhysicsSystem { /* ... */ };
   class RenderSystem { /* ... */ };
   // Ils communiquent via EventBus, pas directement
   ```

3. **Placer logique dans systèmes**
   ```cpp
   // ✅ BON: Position est juste données
   struct Position { float x, y; };
   
   // ❌ MAUVAIS: Position avec logique
   struct Position {
       float x, y;
       void move(float dx, float dy) { x += dx; y += dy; }
   };
   ```

4. **Réutiliser composants**
   ```cpp
   // Un seul componant Position pour joueur, ennemi, bullet, etc.
   // Pas: PlayerPosition, EnemyPosition, BulletPosition
   ```

### ❌ À Éviter

1. **Créer/détruire massivement**
   ```cpp
   // ❌ MAUVAIS: Créer entité chaque frame
   for (int i = 0; i < 100; ++i) {
       world.createEntity();  // Allocation mémoire!
   }

   // ✅ BON: Utiliser object pool
   ```

2. **Dépendances circulaires**
   ```cpp
   // ❌ NON: Système A utilise B, B utilise A
   class SystemA : depends_on<SystemB> {};
   class SystemB : depends_on<SystemA> {};

   // ✅ BON: Utiliser EventBus pour communication
   ```

3. **Logique dans systèmes très lourds**
   ```cpp
   // ❌ NON: Un seul système gigantesque
   class GameSystem : public ISystem { /* 5000 lignes */ };

   // ✅ BON: Décomposer en systèmes spécialisés
   class PhysicsSystem { /* 200 lignes */ };
   class CollisionSystem { /* 150 lignes */ };
   class CombatSystem { /* 180 lignes */ };
   ```

4. **Accès direct aux données internes**
   ```cpp
   // ❌ NON: Accès direct évite les couches d'abstraction
   mgr.getComponentStorage<Position>()[0];  // Accès brut

   // ✅ BON: Utiliser l'API appropriée
   mgr.getComponent<Position>(entity);
   ```

---

## Checklist de Projet

Quand vous créez un jeu:

- [ ] **Analyser** : Identifier tous les éléments (entités, composants, systèmes)
- [ ] **Concevoir composants** : Une responsabilité par composant
- [ ] **Implémenter systèmes** : Logique découplée
- [ ] **Tester** : Unit tests + integration tests
- [ ] **Profiler** : Identifier goulots d'étranglement
- [ ] **Optimiser** : Appliquer patterns selon besoins
- [ ] **Documenter** : Docs pour futurs développeurs
- [ ] **Maintenir** : Respecter conventions

---

## Summary

| Aspect | Bonnes Pratiques |
|--------|-----------------|
| **Components** | Petits, pure data, sérialisables |
| **Systems** | Découplés, une responsabilité, ordre défini |
| **Architecture** | ECS séparant données de logique |
| **Performance** | Profiler, spatial partitioning, batching |
| **Réseau** | Serveur authorité, synchronisation légère |

Le secret d'un jeu scalable: **Simplicité + Séparation des préoccupations + Performance**! 🎮
