# Architecture ECS - SystemManager

## Vue d'ensemble

Le **SystemManager** est un composant central de l'architecture ECS (Entity-Component-System) du moteur R-Type. Il gère le cycle de vie des systèmes et orchestre leur exécution.

## Concepts

### Entity-Component-System (ECS)

L'architecture ECS est un pattern de conception qui sépare les données (Components) de la logique (Systems) :

- **Entity** : Un identifiant unique représentant un objet du jeu
- **Component** : Des données pures sans logique (Position, Velocity, Health, etc.)
- **System** : La logique qui opère sur les entités possédant certains composants

### Avantages de l'ECS

- **Performance** : Cache-friendly, données contiguës en mémoire
- **Flexibilité** : Composition plutôt qu'héritage
- **Maintenabilité** : Séparation claire des responsabilités
- **Testabilité** : Systèmes indépendants facilement testables

## SystemManager

### Responsabilités

Le SystemManager gère :
- L'enregistrement des systèmes
- La récupération des systèmes par type
- La suppression de systèmes
- L'orchestration de l'update de tous les systèmes

### Implémentation

```cpp
class SystemManager {
public:
    // Enregistrer un nouveau système
    template<typename T, typename... Args>
    T* registerSystem(Args&&... args);

    // Récupérer un système par type
    template<typename T>
    T* getSystem();

    // Supprimer un système
    template<typename T>
    void removeSystem();

    // Mettre à jour tous les systèmes
    void update(World& world, float deltaTime);

private:
    std::unordered_map<std::type_index, std::unique_ptr<ISystem>> systems;
};
```

### Stockage des systèmes

Les systèmes sont stockés dans une `std::unordered_map` où :
- **Clé** : `std::type_index` - Identifiant de type runtime
- **Valeur** : `std::unique_ptr<ISystem>` - Ownership unique du système

Cette approche permet :
- Lookup O(1) par type
- Gestion automatique de la mémoire
- Polymorphisme via l'interface ISystem

### Interface ISystem

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(World& world, float deltaTime) = 0;
};
```

Chaque système hérite de `ISystem` et implémente la méthode `update()`.

## Intégration avec World

Le `World` est le conteneur principal qui coordonne le SystemManager et le ComponentManager :

```cpp
class World {
private:
    SystemManager systemManager;
    ComponentManager componentManager;

public:
    // Délègue au SystemManager
    template<typename T, typename... Args>
    T* registerSystem(Args&&... args) {
        return systemManager.registerSystem<T>(std::forward<Args>(args)...);
    }

    // Update tous les systèmes
    void update(float deltaTime) {
        systemManager.update(*this, deltaTime);
    }

    // Accès au ComponentManager pour les systèmes
    ComponentManager& getComponentManager() {
        return componentManager;
    }
};
```

## Exemple d'utilisation

### Créer un système

```cpp
class PhysicsSystem : public ISystem {
private:
    float gravity = 9.81f;

public:
    PhysicsSystem(float customGravity) : gravity(customGravity) {}

    void update(World& world, float deltaTime) override {
        auto& cm = world.getComponentManager();

        // Itérer sur toutes les entités avec Position et Velocity
        for (auto entity : getEntitiesWithComponents<Position, Velocity>()) {
            auto* pos = cm.getComponent<Position>(entity);
            auto* vel = cm.getComponent<Velocity>(entity);

            // Appliquer la gravité
            vel->y += gravity * deltaTime;

            // Mettre à jour la position
            pos->x += vel->x * deltaTime;
            pos->y += vel->y * deltaTime;
        }
    }
};
```

### Enregistrer et utiliser un système

```cpp
// Créer le world
World world;

// Enregistrer les systèmes
world.registerSystem<PhysicsSystem>(9.81f);
world.registerSystem<RenderSystem>();
world.registerSystem<CollisionSystem>();

// Game loop
while (running) {
    float deltaTime = calculateDeltaTime();

    // Update tous les systèmes dans l'ordre d'enregistrement
    world.update(deltaTime);
}
```

## Cycle de vie d'un système

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│  1. Création et enregistrement                      │
│     world.registerSystem<MySystem>(params)          │
│                                                     │
│  2. Récupération (optionnel)                        │
│     auto* sys = world.getSystem<MySystem>()         │
│                                                     │
│  3. Exécution répétée                               │
│     world.update(deltaTime)                         │
│        └─> sys->update(world, deltaTime)            │
│                                                     │
│  4. Suppression (optionnel)                         │
│     world.removeSystem<MySystem>()                  │
│                                                     │
│  5. Destruction automatique                         │
│     Quand World est détruit                         │
│                                                     │
└─────────────────────────────────────────────────────┘
```

## Patterns de conception utilisés

### Template Method Pattern

Le SystemManager utilise des templates pour offrir une API type-safe :

```cpp
// Impossible de mélanger les types
auto* physics = systemManager.getSystem<PhysicsSystem>();
auto* render = systemManager.getSystem<RenderSystem>();
```

### Strategy Pattern

Chaque système est une stratégie différente implémentant l'interface ISystem.

### Registry Pattern

Le SystemManager agit comme un registry centralisé pour tous les systèmes.

## Ordre d'exécution

⚠️ **Important** : Les systèmes sont exécutés dans l'ordre d'enregistrement.

```cpp
// Les systèmes seront exécutés dans cet ordre
world.registerSystem<InputSystem>();      // 1. Traiter les inputs
world.registerSystem<PhysicsSystem>();    // 2. Physique
world.registerSystem<CollisionSystem>();  // 3. Collisions
world.registerSystem<RenderSystem>();     // 4. Rendu
```

Pour changer l'ordre, réenregistrez les systèmes dans le bon ordre ou créez un système de priorités.

## Gestion de la mémoire

### Ownership

- Le SystemManager **possède** les systèmes (unique_ptr)
- Les pointeurs retournés sont **non-owning** (raw pointers)
- Ne pas `delete` un pointeur retourné par `getSystem()`

### Lifetime

```cpp
{
    World world;
    world.registerSystem<MySystem>();

    // Le système existe tant que world existe

} // Le système est automatiquement détruit ici
```

## Performance

### Complexité temporelle

- `registerSystem<T>()` : O(1) - Insertion dans la map
- `getSystem<T>()` : O(1) - Lookup dans la map
- `removeSystem<T>()` : O(1) - Suppression dans la map
- `update()` : O(n) - n = nombre de systèmes

### Optimisations possibles

1. **Dirty flags** : Ne mettre à jour que les systèmes qui ont des données modifiées
2. **Parallel systems** : Exécuter les systèmes indépendants en parallèle
3. **System scheduling** : Ordonnancer les systèmes par priorité/dépendances

## Limitations actuelles

1. **Ordre d'exécution fixe** : Pas de système de priorités
2. **Mono-thread** : Exécution séquentielle
3. **Pas de dépendances explicites** : Les systèmes doivent gérer leurs dépendances

## Extensions futures possibles

### Système de priorités

```cpp
world.registerSystem<PhysicsSystem>(Priority::High);
world.registerSystem<RenderSystem>(Priority::Low);
```

### Systèmes parallèles

```cpp
// Exécuter ces systèmes en parallèle (pas de dépendances)
world.parallelUpdate({
    getSystem<PhysicsSystem>(),
    getSystem<AISystem>()
});
```

### Système d'événements

```cpp
class EventSystem : public ISystem {
    void onCollision(Entity a, Entity b) { /* ... */ }
};
```

## Références

- [ECS FAQ](https://github.com/SanderMertens/ecs-faq)
- [Overwatch Gameplay Architecture](https://www.youtube.com/watch?v=W3aieHjyNvw)
- [Understanding Component-Entity-Systems](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/understanding-component-entity-systems-r3013/)

## Voir aussi

- [Guide de tests](../guides/TESTING.md) - Tests du SystemManager
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Architecture globale du projet
- [CODING_STANDARDS.md](../CODING_STANDARDS.md) - Standards de code
