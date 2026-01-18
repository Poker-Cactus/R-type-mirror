# Documentation Technique - R-Type Engine

## 📚 Table des Matières

### Fondamentaux ECS
- **[ECS Engine Complet](ECS-Engine.md)** - Guide complet du système ECS
  - Concepts fondamentaux
  - Architecture détaillée
  - Exemples pratiques
  - Bonnes pratiques

### Gestion des Systèmes
- **[System Manager](SYSTEM_MANAGER.md)** - Gestion des systèmes dans le moteur
  - Enregistrement des systèmes
  - Ordre d'exécution
  - Événements système

### Architecture Réseau
- **[Protocole Réseau](../PROTOCOL.md)** - Communication client-serveur
  - Paquets et sérialisation
  - Synchronisation d'état
  - Gestion des connexions

### Fonctionnalités Spécialisées
- **[Sprites Côté Serveur](../SERVER_DRIVEN_SPRITES.md)** - Gestion des ressources graphiques
  - Configuration serveur
  - Synchronisation sprites
  - Optimisation réseau

---

## 🎯 Parcours par Sujet

### Apprendre l'ECS (Beginners)
```
1. Lire: ECS Engine (concepts fondamentaux)
2. Lire: guides/Creating-A-Game.md
3. Faire: Exécuter Examples
```

### Implémenter un Système
```
1. Lire: ECS Engine (architecture détaillée)
2. Lire: guides/Creating-Systems.md
3. Lire: Code Examples
4. Implémenter votre système
```

### Architecture Réseau
```
1. Lire: PROTOCOL.md
2. Lire: guides/Architecture-Best-Practices.md (Client vs Serveur)
3. Lire: Code Examples (Example 3)
4. Implémenter synchronisation
```

---

## 📖 Quick Reference

### Components

```cpp
// Créer un composant
struct Position {
    float x, y;
};

// Ajouter à entité
world.addComponent(entity, Position{10, 20});

// Accéder
auto& pos = mgr.getComponent<Position>(entity);

// Vérifier présence
if (mgr.hasComponent<Position>(entity)) { }

// Supprimer
mgr.removeComponent<Position>(entity);
```

### Systems

```cpp
// Créer un système
class MySystem : public ISystem {
public:
    ComponentSignature getSignature() const override {
        ComponentSignature sig;
        sig.set(getComponentId<Position>());
        sig.set(getComponentId<Velocity>());
        return sig;
    }

    void update(World& world, float deltaTime) override {
        // Logique
    }
};

// Enregistrer
world.registerSystem<MySystem>();

// Exécuter
world.update(deltaTime);
```

### Événements

```cpp
// Définir événement
struct MyEvent {
    int data;
};

// Émettre
world.getEventBus().emit<MyEvent>(42);

// Écouter
world.getEventBus().subscribe<MyEvent>(
    [](const MyEvent& evt) {
        std::cout << evt.data << std::endl;
    }
);
```

---

## 🏗️ Architecture Globale

```
┌─────────────────────────────────────┐
│        engineCore (Library)         │
├─────────────────────────────────────┤
│                                     │
│  ┌─────────────┐  ┌─────────────┐  │
│  │   World     │  │  EventBus   │  │
│  └─────┬───────┘  └─────────────┘  │
│        │                            │
│  ┌─────┴──────────────────────┐    │
│  │  EntityManager             │    │
│  │  ComponentManager          │    │
│  │  SystemManager             │    │
│  └────────────────────────────┘    │
│                                     │
└─────────────────────────────────────┘
         ↑              ↑
    ┌────┘          ┌───┘
    │               │
┌───┴────┐      ┌───┴────┐
│ SERVER  │      │ CLIENT  │
├─────────┤      ├─────────┤
│ Systems │      │ Systems │
│ Network │      │ Render  │
└─────────┘      └─────────┘
```

---

## ⚙️ Cycle de Vie

### Initialisation
```cpp
1. Créer World
2. Enregistrer Systèmes (ordre important)
3. Créer Entités
4. Ajouter Composants
```

### Boucle de Jeu
```cpp
1. World.update(deltaTime)
   a. Pour chaque System (dans l'ordre):
      - Récupérer entités avec signature
      - Appliquer logique
   b. Émettre/traiter événements
2. Post-processing
3. Rendu
```

### Shutdown
```cpp
1. Détruire toutes les entités
2. Libérer ressources
3. Quitter World
```

---

## 📊 Performance

### Complexité

| Opération | Complexité |
|-----------|-----------|
| Créer entité | O(1) |
| Ajouter composant | O(1) amortized |
| Accéder composant | O(1) |
| Filtering par signature | O(1)* |
| Itérer entités | O(n) |
| Détruire entité | O(1) + composants |

*Bitwise AND operation, très rapide

### Optimisations

1. **Spatial Partitioning** - QuadTree pour collisions
2. **Batching** - Grouper draw calls
3. **Object Pool** - Réutiliser entités
4. **Parallel Systems** - C++17 execution policies
5. **Cache Locality** - Ordonner systèmes intelligemment

---

## 🔧 Troubleshooting

### Problème: "Entité n'est pas trouvée"
```cpp
// Vérifier si entité existe
if (!world.isAlive(entity)) {
    std::cerr << "Entité détruite!" << std::endl;
}
```

### Problème: "Composant introuvable"
```cpp
// Toujours vérifier avant accès
if (mgr.hasComponent<Position>(entity)) {
    auto& pos = mgr.getComponent<Position>(entity);
}
```

### Problème: "Performance faible"
```cpp
1. Profiler avec chrono
2. Réduire itérations (signatures)
3. Utiliser spatial partitioning
4. Paralléliser si possible
5. Réduire allocations
```

---

## 📚 Ressources

### Documentation Interne
- [guides/Creating-A-Game.md](../guides/Creating-A-Game.md)
- [guides/Creating-Systems.md](../guides/Creating-Systems.md)
- [guides/Adding-Components.md](../guides/Adding-Components.md)
- [guides/Code-Examples.md](../guides/Code-Examples.md)

### References ECS Externes
- [EnTT Documentation](https://skypjack.github.io/entt/)
- [Bevy ECS](https://docs.rs/bevy_ecs/latest/bevy_ecs/)
- [Game Programming Patterns](https://gameprogrammingpatterns.com/contents.html)

---

**Version:** 2.0  
**Updated:** 2025-01-18  
**Maintainer:** R-Type Development Team
