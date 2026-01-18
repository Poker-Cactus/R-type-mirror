# Guides de Développement R-Type

Bienvenue dans les guides de développement! Ici vous trouverez tout ce dont vous avez besoin pour créer un jeu avec le moteur R-Type.

## 🚀 Par Où Commencer?

### Pour les Nouveaux Développeurs

1. **[Créer un Nouveau Jeu](Creating-A-Game.md)** ⭐ **START HERE**
   - Vue d'ensemble complète du processus
   - Exemple pas-à-pas d'un jeu simple
   - Checklist de vérification

2. **[Comprendre l'ECS](../technical/ECS-Engine.md)**
   - Concepts fondamentaux (Entity, Component, System)
   - Comment fonctionne l'architecture ECS
   - Patterns et bonnes pratiques

3. **[Créer des Composants](Adding-Components.md)**
   - Comment structurer les données
   - Signatures et filtering efficace
   - Sérialisation pour le réseau

4. **[Implémenter les Systèmes](Creating-Systems.md)**
   - Cycles de vie des systèmes
   - Systèmes de base (Physics, Collision, Combat)
   - Patterns avancés

### Pour les Architectes

5. **[Architecture & Best Practices](Architecture-Best-Practices.md)**
   - Design patterns (Factory, Pool, Events)
   - Séparation Client/Serveur
   - Performance et optimisation
   - Anti-patterns à éviter

### Exemples Pratiques

6. **[Code Examples](Code-Examples.md)**
   - 5 exemples complets et fonctionnels
   - De simple 2D à systèmes complexes
   - Code prêt à copier-coller

### Standards & Conventions

7. **[Conventions de Code](Code-Conventions.md)** *À venir*
   - Nommage et organisation
   - Formatage
   - Commentaires et documentation

---

## 📚 Parcours d'Apprentissage

### Niveau 1: Débutant (1-2 jours)
```
1. Lire "Créer un Nouveau Jeu"
2. Comprendre l'ECS
3. Exécuter l'Example 1 (Jeu 2D simple)
4. Créer votre propre jeu simple
```

### Niveau 2: Intermédiaire (3-5 jours)
```
1. Créer des Composants avancés
2. Implémenter plusieurs Systèmes
3. Utiliser les Événements
4. Exécuter Examples 2-3
```

### Niveau 3: Avancé (1 semaine+)
```
1. Architecture complète
2. Optimisation
3. Synchronisation réseau
4. Pattern Design avancés
5. Système d'animation/IA
```

---

## 🎯 Guides par Cas d'Usage

### Je veux créer...

#### Un Jeu Plateforme 2D
→ [Créer un Nouveau Jeu](Creating-A-Game.md) + [Example 1](Code-Examples.md#example-1--jeu-simple-2d)

#### Un Jeu d'Action avec Combat
→ [Example 2: Système de Combat](Code-Examples.md#example-2--système-de-combat)

#### Un Jeu Multijoueur
→ [Example 3: Gestion Réseau](Code-Examples.md#example-3--gestion-réseau) + [Architecture](Architecture-Best-Practices.md#client-vs-serveur)

#### Un Jeu avec Animations
→ [Example 4: Animation et Effets](Code-Examples.md#example-4--animation-et-effets)

#### Optimiser les Performances
→ [Architecture: Performance & Optimisation](Architecture-Best-Practices.md#performance--optimisation)

---

## 📖 Organisation des Documents

```
guides/
├── Creating-A-Game.md                    # Vue d'ensemble projet
├── Adding-Components.md                  # Structure des données
├── Creating-Systems.md                   # Logique du jeu
├── Architecture-Best-Practices.md        # Patterns & optimisation
├── Code-Examples.md                      # Exemples complets
├── Code-Conventions.md                   # Standards (à venir)
└── README.md                             # Ce fichier
```

---

## 🔑 Concepts Clés

### Entity-Component-System (ECS)

| Concept | Rôle | Exemple |
|---------|------|---------|
| **Entity** | Conteneur identifier unique | Joueur, Ennemi, Projectile |
| **Component** | Données pures | Position, Velocity, Health |
| **System** | Logique métier | PhysicsSystem, RenderSystem |
| **Signature** | Filtre d'entités | Requiert Position + Velocity |

### Workflow Typique

```
1. Analyser le jeu
   ↓
2. Définir Composants
   ↓
3. Implémenter Systèmes
   ↓
4. Tester & Itérer
   ↓
5. Optimiser si nécessaire
```

---

## ⚠️ Anti-Patterns à Éviter

### ❌ Ne Pas Faire

```cpp
// MAUVAIS: Logique dans les composants
struct BadHealth {
    int hp;
    void takeDamage(int dmg) { hp -= dmg; }  // ❌
};

// MAUVAIS: Systèmes couplés
class PhysicsSystem {
    void update() {
        renderer.draw();  // ❌ Trop de responsabilités
    }
};

// MAUVAIS: Créer/détruire massivement
for (int i = 0; i < 1000; ++i) {
    world.createEntity();  // ❌ Très inefficace
}
```

### ✅ À Faire

```cpp
// BON: Composants = données pures
struct Health {
    int hp = 100;
    int maxHp = 100;
};

// BON: Systèmes découplés
class HealthSystem : public ISystem { /* ... */ };
class PhysicsSystem : public ISystem { /* ... */ };

// BON: Object pool pour réutilisation
ProjectilePool pool(100);
auto proj = pool.acquire(x, y);  // Réutilisé
```

---

## 🛠️ Outils & Resources

### Internes
- [Registry API](../api/Registry.md) - API du World
- [Component Manager](../api/ComponentManager.md) - Gestion composants
- [System Manager](../api/SystemManager.md) - Gestion systèmes
- [Event Bus](../api/EventBus.md) - Système d'événements

### Externes
- [EnTT (Reference ECS)](https://github.com/skypjack/entt) - Inspiration
- [Bevy Engine Docs](https://bevyengine.org/) - Resource ECS
- [Game Architecture Patterns](https://gameprogrammingpatterns.com/) - Patterns

---

## 🐛 Débogage

### Affichage de Debug

```cpp
class DebugSystem : public ISystem {
    void update(World& world, float dt) override {
        std::cout << "Entities: " << world.getEntityCount() << std::endl;
        
        auto& mgr = world.getComponentManager();
        std::cout << "Positions: " 
                  << mgr.getEntitiesWithComponents<Position>().size()
                  << std::endl;
    }
};
```

### Profiling

```cpp
auto start = std::chrono::high_resolution_clock::now();
world.update(deltaTime);
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Frame time: " << duration.count() << " µs" << std::endl;
```

---

## ❓ Questions Fréquentes

### Q: Par où commencer?
**A:** Lisez [Créer un Nouveau Jeu](Creating-A-Game.md), puis exécutez l'[Example 1](Code-Examples.md#example-1--jeu-simple-2d).

### Q: Comment optimiser mon jeu?
**A:** Voir [Performance & Optimisation](Architecture-Best-Practices.md#performance--optimisation).

### Q: Comment faire du multijoueur?
**A:** Voir [Client vs Serveur](Architecture-Best-Practices.md#client-vs-serveur) et [Example 3](Code-Examples.md#example-3--gestion-réseau).

### Q: Comment créer une animation?
**A:** Voir [Example 4](Code-Examples.md#example-4--animation-et-effets).

### Q: Quelle est la meilleure pratique pour...?
**A:** Consulter [Architecture & Best Practices](Architecture-Best-Practices.md).

---

## 📞 Support

- **Questions?** Consultez les FAQs ci-dessus
- **Bug/Amélioration?** Ouvrez une [issue](https://github.com/Poker-Cactus/R-type-mirror/issues)
- **Documentation?** Contribuez au [wiki](https://github.com/Poker-Cactus/R-type-mirror/wiki)

---

**Happy Game Making! 🎮**
