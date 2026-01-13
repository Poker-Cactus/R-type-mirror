# 🎮 Quick Start - Enemy Configuration System

## 🚀 Créer un nouvel ennemi en 3 étapes

### 1️⃣ Ouvrir le fichier de configuration
```bash
nano server/config/enemies.json
```

### 2️⃣ Ajouter votre ennemi dans le tableau `"enemies"`
```json
{
  "id": "mon_ennemi",
  "name": "Mon Super Ennemi",
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
    "hp": 50,
    "maxHp": 50
  },
  "collider": {
    "width": 99.0,
    "height": 108.0
  },
  "velocity": {
    "dx": -400.0,
    "dy": 0.0
  },
  "pattern": {
    "type": "sine_wave",
    "amplitude": 50.0,
    "frequency": 5.0
  },
  "spawn": {
    "groupSizeMin": 2,
    "groupSizeMax": 6,
    "spawnDelay": 0.4,
    "spawnInterval": 7.0
  }
}
```

### 3️⃣ Utiliser dans le jeu
```cpp
// Dans Game.cpp ou Lobby.cpp, après le chargement :
spawnSystem->setCurrentEnemyType("mon_ennemi");
```

## 🎯 Ennemis pré-configurés

### `enemy_red` - Standard
- **HP**: 30
- **Vitesse**: -384
- **Pattern**: Sine wave (amplitude 40)
- **Groupes**: 1-5 ennemis

### `enemy_blue` - Rapide
- **HP**: 20
- **Vitesse**: -300
- **Pattern**: Sine wave large (amplitude 60)
- **Groupes**: 2-4 ennemis

### `enemy_fast` - Scout
- **HP**: 15
- **Vitesse**: -600
- **Pattern**: Ligne droite
- **Groupes**: 3-7 ennemis

### `enemy_tank` - Tank
- **HP**: 100
- **Vitesse**: -150
- **Pattern**: Sine wave lente
- **Groupes**: 1-2 ennemis

## ⚙️ Valeurs recommandées

### Vitesse (`velocity.dx`)
- 🐢 Lent: -150 à -250
- 🏃 Normal: -300 à -400
- 🚀 Rapide: -500 à -700

### Points de vie (`health.hp`)
- 💨 Fragile: 10-20
- 💪 Normal: 20-40
- 🛡️ Tank: 50-150

### Pattern sine_wave
- **amplitude**: Hauteur de l'oscillation (20-80)
- **frequency**: Vitesse de l'oscillation (2-8)

### Spawn
- **groupSizeMin/Max**: Taille des groupes (1-10)
- **spawnDelay**: Délai entre ennemis (0.1-1.0s)
- **spawnInterval**: Délai entre groupes (3.0-15.0s)

### Scale & Collider
```
collider.width = sprite.width * transform.scale
collider.height = sprite.height * transform.scale
```

## 🔄 Cycle d'ennemis (pour les vagues)

```cpp
// Changer automatiquement d'ennemi à chaque vague
spawnSystem->cycleEnemyType();

// Ou définir manuellement
spawnSystem->setCurrentEnemyType("enemy_tank");  // Vague de tanks !
```

## 📊 Exemple : Progression de difficulté

```cpp
// Vague 1: Ennemis standards
spawnSystem->setCurrentEnemyType("enemy_red");

// Après 30 secondes
spawnSystem->setCurrentEnemyType("enemy_blue");

// Après 60 secondes
spawnSystem->setCurrentEnemyType("enemy_fast");

// Boss à 90 secondes
spawnSystem->setCurrentEnemyType("enemy_tank");
```

## 🐛 Problèmes courants

### ❌ Ennemis ne spawnen pas
- Vérifier que `enemies.json` existe
- Valider le JSON: `python3 -m json.tool server/config/enemies.json`

### ❌ Collisions incorrectes
- Vérifier que collider correspond à sprite × scale
- Exemple: `sprite 33×36` avec `scale 3.0` = collider `99×108`

### ❌ Animation ne joue pas
- Vérifier que `animated = true`
- `startFrame` doit être différent de `endFrame`
- `frameTime` > 0

## 💡 Tips

✨ **Tester rapidement**: Modifier le JSON et relancer le serveur (pas besoin de recompiler)  
✨ **Backup**: Copier `enemies.json` avant de modifier  
✨ **Créatif**: Essayer des valeurs extrêmes pour des effets fun  
✨ **Balance**: Ajuster les valeurs en jouant pour équilibrer le jeu  

## 📚 Documentation complète

Voir [`server/config/README.md`](README.md) pour la documentation complète.
