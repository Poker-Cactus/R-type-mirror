# R-Type Game Systems Architecture

## 📋 Vue d'ensemble

Cette architecture implémente un système ECS (Entity-Component-System) event-driven pour le jeu R-Type, avec une séparation claire entre le serveur (autorité) et le client (rendu).

## 🎯 Rôles

### Serveur (Autorité)
- ✅ Exécute toute la logique de gameplay
- ✅ Applique tous les systèmes (Physique, Collisions, IA, Tir...)
- ✅ Maintient l'état officiel du monde
- ✅ Envoie des snapshots aux clients
- ✅ Gère le spawn des entités

### Client  
- ✅ Rend l'image (SDL2)
- ✅ Lit l'input joueur
- ✅ Peut faire des prédictions (optionnel)
- ✅ Affiche l'état reçu du serveur
- ✅ Interpole/extrapole
- ✅ Envoie les commandes joueur (pas la logique!)

### Moteur ECS (Commun)
- ✅ ComponentManager
- ✅ SystemManager
- ✅ World
- ✅ EventBus
- ✅ Composants
- ✅ Systèmes génériques

## 🧩 Composants (Data Only)

### Composants de base
```cpp
Transform    // Position, rotation, scale
Velocity     // Déplacement
Health       // Points de vie
Input        // Entrées joueur
Sprite       // Rendu visuel
Collider     // Détection de collision
```

### Tags
```cpp
PlayerTag       // Identifie les joueurs
EnemyTag        // Identifie les ennemis (+ type et score)
ProjectileTag   // Identifie les projectiles (+ owner et damage)
NetworkOwned    // Propriété réseau
Lifetime        // Durée de vie temporaire
```

## ⚙️ Systèmes (Logique)

### Systèmes de gameplay (Serveur)

#### 1. **MovementSystem** ✅
- Met à jour `Transform` selon `Velocity`
- Appliqué à toutes les entités avec Transform + Velocity

#### 2. **CollisionSystem** ✅
- Détecte les collisions entre entités
- Supporte BOX et CIRCLE
- Émet `CollisionEvent`

#### 3. **DamageSystem** ✅
- Écoute `CollisionEvent`
- Applique les dégâts selon les tags
- Émet `DamageEvent` et `DeathEvent`

#### 4. **DeathSystem** ✅
- Détruit les entités avec hp <= 0
- Écoute `DeathEvent` pour effets spéciaux
- Émet `ScoreEvent` quand ennemi meurt

#### 5. **ShootingSystem** ✅
- Lit `Input.shoot`
- Gère le cooldown de tir
- Émet `ShootEvent`
- Écoute `ShootEvent` pour spawner projectiles

#### 6. **EnemyAISystem** ✅
- Contrôle le comportement des ennemis
- Patterns de mouvement selon le type
- Tir automatique
- Émet `EnemyAIEvent` et `ShootEvent`

#### 7. **SpawnSystem** ✅
- Écoute `SpawnEntityEvent`
- Créé les entités (ennemis, projectiles, powerups)
- Spawn automatique d'ennemis périodique

#### 8. **LifetimeSystem** ✅
- Détruit les entités après leur durée de vie
- Utilisé pour projectiles, effets temporaires

### Systèmes réseau (À implémenter)

#### 9. **NetworkServerSystem** 🔨
```cpp
- Sérialise l'état du monde
- Envoie snapshots aux clients
- Écoute PlayerInputEvent
- Émet SendSnapshotEvent
```

#### 10. **NetworkClientSystem** 🔨
```cpp
- Reçoit les snapshots
- Met à jour les composants locaux
- Émet SnapshotReceivedEvent
- Interpole les positions
```

## 📡 Événements (Communication)

### Événements de gameplay
```cpp
CollisionEvent      // entityA, entityB collided
DamageEvent         // target took damage from source
DeathEvent          // entity died, killed by killer
ShootEvent          // entity shot in direction
SpawnEntityEvent    // spawn entity at position
EnemyAIEvent        // enemy AI decision
ScoreEvent          // player gained points
```

### Événements réseau
```cpp
SnapshotReceivedEvent  // Server state received
SendSnapshotEvent      // Request to send snapshot
PlayerInputEvent       // Player input received
```

## 🔄 Flow d'exécution

### Serveur (60 FPS)
```
1. Poll network input → PlayerInputEvent
2. Update MovementSystem
3. Update EnemyAISystem → ShootEvent
4. Update ShootingSystem → SpawnEntityEvent
5. Update SpawnSystem (listen events)
6. Update CollisionSystem → CollisionEvent
7. Update DamageSystem → DamageEvent, DeathEvent
8. Update DeathSystem → ScoreEvent
9. Update LifetimeSystem
10. Serialize world → SendSnapshotEvent
11. NetworkServerSystem sends to clients
```

### Client (60 FPS)
```
1. Read keyboard/mouse → send to server
2. Receive snapshot → SnapshotReceivedEvent
3. Update local components
4. Interpolate positions
5. Render (SDL2)
```

## 📝 Exemple d'utilisation

### Initialisation du serveur
```cpp
Game game;

// Les systèmes sont automatiquement enregistrés
world->registerSystem<MovementSystem>();
world->registerSystem<CollisionSystem>();
world->registerSystem<DamageSystem>();
// etc...

// Initialiser les systèmes event-driven
damageSystem->initialize(*world);
shootingSystem->initialize(*world);
spawnSystem->initialize(*world);

// Spawn player
Entity player = world->createEntity();
world->addComponent(player, Transform{100, 300});
world->addComponent(player, Health{100, 100});
world->addComponent(player, PlayerTag{0});

game.runGameLoop();
```

### Interaction via événements
```cpp
// Système A émet un événement
CollisionEvent event(entityA, entityB);
world.emitEvent(event);

// Système B écoute l'événement
world.subscribeEvent<CollisionEvent>([](const CollisionEvent& e) {
    // Appliquer les dégâts
    applyDamage(e.entityA, e.entityB);
});
```

## 🚀 Prochaines étapes

1. ✅ Implémenter NetworkServerSystem
2. ✅ Implémenter NetworkClientSystem
3. ✅ Ajouter interpolation client-side
4. ✅ Ajouter prédiction client-side
5. ✅ Implémenter le système de scoring
6. ✅ Ajouter les powerups
7. ✅ Implémenter les effets visuels (explosions)
8. ✅ Ajouter le système de son

## 📚 Fichiers clés

```
engineCore/
├── include/ecs/
│   ├── World.hpp              # Coordinateur central
│   ├── events/
│   │   ├── EventBus.hpp       # Message bus
│   │   ├── IEvent.hpp         # Interface événement
│   │   └── GameEvents.hpp     # Tous les événements
│   ├── components/
│   │   ├── Transform.hpp
│   │   ├── Velocity.hpp
│   │   ├── Health.hpp
│   │   ├── Input.hpp
│   │   ├── Collider.hpp
│   │   └── Tags.hpp
│   └── systems/
│       └── MovementSystem.hpp

server/
├── include/
│   ├── Game.hpp               # Boucle principale
│   └── systems/
│       ├── CollisionSystem.hpp
│       ├── DamageSystem.hpp
│       ├── DeathSystem.hpp
│       ├── ShootingSystem.hpp
│       ├── EnemyAISystem.hpp
│       ├── SpawnSystem.hpp
│       └── LifetimeSystem.hpp
└── src/
    ├── main.cpp
    └── Game.cpp
```

## 💡 Principes de conception

1. **Composants = Data pure** (pas de logique)
2. **Systèmes = Logique pure** (pas d'état partagé)
3. **Événements = Communication asynchrone**
4. **Serveur = Autorité absolue**
5. **Client = Rendu optimiste**

---

**EPITECH PROJECT © 2025**
