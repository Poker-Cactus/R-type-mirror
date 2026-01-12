# Enemy Configuration System

Ce système permet de créer et modifier des ennemis via des fichiers de configuration JSON, sans toucher au code source.

## 📁 Structure

```
server/
  ├── config/
  │   └── enemies.json          # Fichier de configuration des ennemis
  ├── include/
  │   └── config/
  │       └── EnemyConfig.hpp   # Header du système de config
  └── src/
      └── config/
          └── EnemyConfig.cpp   # Implémentation du loader
```

## 📝 Format du fichier JSON

```json
{
  "enemies": [
    {
      "id": "enemy_red",
      "name": "Red Oscillator",
      "sprite": {
        "spriteId": 2,
        "width": 33,
        "height": 36,
        "animated": true,
        "frameCount": 16,
        "startFrame": 7,
        "endFrame": 0,
        "frameTime": 0.1,
        "reverseAnimation": true
      },
      "transform": {
        "scale": 3.0
      },
      "health": {
        "hp": 30,
        "maxHp": 30
      },
      "collider": {
        "width": 99.0,
        "height": 108.0
      },
      "velocity": {
        "dx": -384.0,
        "dy": 0.0
      },
      "pattern": {
        "type": "sine_wave",
        "amplitude": 40.0,
        "frequency": 6.0
      },
      "spawn": {
        "groupSizeMin": 1,
        "groupSizeMax": 5,
        "spawnDelay": 0.3,
        "spawnInterval": 6.0
      }
    }
  ]
}
```

## 🎮 Propriétés configurables

### `id` (string, requis)
Identifiant unique de l'ennemi. Utilisé pour spawner ce type d'ennemi.

### `name` (string, optionnel)
Nom descriptif de l'ennemi (pour la documentation).

### `sprite` (object)
- `spriteId`: ID du sprite (voir `ecs::SpriteId`)
- `width`, `height`: Dimensions d'une frame du sprite
- `animated`: Si true, le sprite est animé
- `frameCount`: Nombre total de frames
- `startFrame`: Frame de début de l'animation
- `endFrame`: Frame de fin de l'animation
- `frameTime`: Durée d'une frame en secondes
- `reverseAnimation`: Si true, joue l'animation en arrière

### `transform` (object)
- `scale`: Échelle de l'ennemi (1.0 = taille normale)

### `health` (object)
- `hp`: Points de vie initiaux
- `maxHp`: Points de vie maximum

### `collider` (object)
- `width`: Largeur du collider
- `height`: Hauteur du collider

**💡 Conseil**: Calculer à partir des dimensions du sprite × scale

### `velocity` (object)
- `dx`: Vélocité horizontale (négatif = vers la gauche)
- `dy`: Vélocité verticale (négatif = vers le haut)

### `pattern` (object)
- `type`: Type de mouvement (`"sine_wave"`, `"zigzag"`, `"none"`, etc.)
- `amplitude`: Amplitude du mouvement oscillatoire
- `frequency`: Fréquence du mouvement

### `spawn` (object)
- `groupSizeMin`: Taille minimale d'un groupe d'ennemis
- `groupSizeMax`: Taille maximale d'un groupe
- `spawnDelay`: Délai entre chaque ennemi d'un groupe (en secondes)
- `spawnInterval`: Intervalle entre chaque groupe (en secondes)

## 🚀 Utilisation dans le code

### Chargement automatique

Le système charge automatiquement `server/config/enemies.json` au démarrage :

```cpp
// Game.cpp - Fait automatiquement
m_enemyConfigManager = std::make_shared<server::EnemyConfigManager>();
if (m_enemyConfigManager->loadFromFile("server/config/enemies.json")) {
    spawnSystem->setEnemyConfigManager(m_enemyConfigManager);
}
```

### Spawner un ennemi depuis la config

```cpp
// Spawner un groupe d'ennemis "enemy_red"
spawnSystem->spawnEnemyGroup(world, "enemy_red");

// Spawner un seul ennemi "enemy_blue" à une position
spawnSystem->spawnEnemyFromConfig(world, 750.0f, 300.0f, "enemy_blue");
```

### Changer le type d'ennemi courant

```cpp
// Définir manuellement
spawnSystem->setCurrentEnemyType("enemy_blue");

// Cycler vers le type suivant (utile pour les vagues)
spawnSystem->cycleEnemyType();

// Obtenir le type actuel
std::string currentType = spawnSystem->getCurrentEnemyType();
```

### Accéder directement à la config

```cpp
const EnemyConfig* config = m_enemyConfigManager->getConfig("enemy_red");
if (config) {
    std::cout << "Enemy: " << config->name << std::endl;
    std::cout << "HP: " << config->health.hp << std::endl;
}

// Liste tous les types d'ennemis disponibles
std::vector<std::string> enemyIds = m_enemyConfigManager->getEnemyIds();
for (const auto& id : enemyIds) {
    std::cout << "Available: " << id << std::endl;
}
```

## 📊 Exemple : Créer un nouvel ennemi

### 1. Ajouter dans `enemies.json`

```json
{
  "id": "enemy_fast",
  "name": "Fast Scout",
  "sprite": {
    "spriteId": 2,
    "width": 33,
    "height": 36,
    "animated": true,
    "frameCount": 16,
    "startFrame": 0,
    "endFrame": 15,
    "frameTime": 0.05,
    "reverseAnimation": false
  },
  "transform": {
    "scale": 2.0
  },
  "health": {
    "hp": 15,
    "maxHp": 15
  },
  "collider": {
    "width": 66.0,
    "height": 72.0
  },
  "velocity": {
    "dx": -600.0,
    "dy": 0.0
  },
  "pattern": {
    "type": "zigzag",
    "amplitude": 80.0,
    "frequency": 8.0
  },
  "spawn": {
    "groupSizeMin": 3,
    "groupSizeMax": 7,
    "spawnDelay": 0.2,
    "spawnInterval": 4.0
  }
}
```

### 2. L'utiliser immédiatement

```cpp
// Aucun code à recompiler !
spawnSystem->setCurrentEnemyType("enemy_fast");
```

### 3. Relancer le serveur

```bash
./build/server/server
# [Game] Enemy configurations loaded successfully
# [EnemyConfig] Loaded enemy: enemy_red (Red Oscillator)
# [EnemyConfig] Loaded enemy: enemy_blue (Blue Zigzag)
# [EnemyConfig] Loaded enemy: enemy_fast (Fast Scout)  ← Nouveau !
```

## 🔧 Patterns de mouvement disponibles

- `"sine_wave"`: Oscillation sinusoïdale verticale
- `"zigzag"`: Zigzag (à implémenter dans EnemyAISystem)
- `"circle"`: Mouvement circulaire (à implémenter)
- `"none"`: Mouvement linéaire simple

**💡 Astuce**: Pour ajouter un nouveau pattern, modifiez `EnemyAISystem.hpp`

## ⚠️ Fallback

Si le fichier de configuration n'est pas trouvé ou contient des erreurs :
- Le système utilise les ennemis hardcodés (`spawnEnemyRed`)
- Un message d'avertissement est affiché dans la console
- Le jeu continue de fonctionner normalement

## 🎯 Avantages

✅ **Aucune recompilation** nécessaire pour modifier un ennemi  
✅ **Expérimentation rapide** : tester différentes valeurs sans rebuild  
✅ **Game designers** peuvent tweaker sans toucher au code  
✅ **Versionnable** : historique des changements via Git  
✅ **Facile à étendre** : ajout de nouveaux ennemis en 2 minutes  
✅ **Validation** : le système vérifie la validité du JSON  

## 🐛 Debugging

### Vérifier que la config est chargée

```bash
# Le serveur doit afficher au démarrage :
[Game] Enemy configurations loaded successfully
[EnemyConfig] Loaded enemy: enemy_red (Red Oscillator)
[EnemyConfig] Loaded enemy: enemy_blue (Blue Zigzag)
```

### Si aucun ennemi n'apparaît

1. Vérifier que le fichier existe : `ls server/config/enemies.json`
2. Valider le JSON : `python3 -m json.tool server/config/enemies.json`
3. Vérifier les logs du serveur pour les erreurs

### Tester un ennemi spécifique

```cpp
// Dans Game.cpp, après le chargement :
if (spawnSystem && m_enemyConfigManager) {
    spawnSystem->setCurrentEnemyType("enemy_blue");  // Force l'utilisation de enemy_blue
}
```

## 📚 Prochaines étapes

- [ ] Ajouter d'autres patterns (spiral, zigzag, cercle)
- [ ] Support des ennemis avec comportements de tir
- [ ] Système de vagues avec progression de difficulté
- [ ] Éditeur visuel pour créer des ennemis (optionnel)
- [ ] Hot-reload : recharger la config sans redémarrer le serveur
