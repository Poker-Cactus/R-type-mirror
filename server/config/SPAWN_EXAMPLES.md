# 🎮 Exemples d'utilisation - Spawn d'ennemis

## 📋 3 façons de spawner plusieurs types d'ennemis

### ✅ Option 1 : Spawn simultané (RECOMMANDÉ)

Chaque type d'ennemi spawn selon son propre intervalle défini dans `enemies.json`.

```cpp
// Dans Game.cpp, après le chargement de la config
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
```

**Résultat** :
- Les rouges spawnen toutes les 6 secondes (leur `spawnInterval`)
- Les bleus spawnen toutes les 8 secondes (leur `spawnInterval`)
- Les deux types apparaissent en parallèle ! 🔥

---

### ✅ Option 2 : Alternance automatique

Un seul type à la fois, mais change automatiquement après chaque spawn.

```cpp
// Dans Game.cpp
spawnSystem->setCurrentEnemyType("enemy_red");
// cycleEnemyType() est appelé automatiquement
```

**Résultat** :
- Groupe de rouges → Groupe de bleus → Groupe de rouges → etc.

---

### ✅ Option 3 : Contrôle manuel

Tu décides quand changer de type.

```cpp
// Spawner des rouges pendant 30 secondes
spawnSystem->setCurrentEnemyType("enemy_red");

// Puis basculer sur des bleus
spawnSystem->setCurrentEnemyType("enemy_blue");

// Ou cycler vers le type suivant
spawnSystem->cycleEnemyType();
```

---

## 🎯 Exemples concrets

### Exemple 1 : Rouges + Bleus en parallèle

```cpp
// Game.cpp - après loadFromFile
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
```

**Ce qui se passe** :
```
t=0s  : Groupe rouge (1-5 ennemis)
t=6s  : Groupe rouge
t=8s  : Groupe bleu (2-4 ennemis)
t=12s : Groupe rouge
t=16s : Groupe bleu + Groupe rouge
...
```

---

### Exemple 2 : Tous les types en même temps

```cpp
spawnSystem->enableMultipleSpawnTypes({
    "enemy_red",    // Toutes les 6s
    "enemy_blue",   // Toutes les 8s
    "enemy_fast",   // Toutes les 4s
    "enemy_tank"    // Toutes les 12s
});
```

**Chaos total** ! 🎉

---

### Exemple 3 : Progression de difficulté

```cpp
// Début du jeu
spawnSystem->setCurrentEnemyType("enemy_red");

// Après 30 secondes
spawnSystem->setCurrentEnemyType("enemy_blue");

// Après 60 secondes - Activer le mode parallèle
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});

// Après 90 secondes - Ajouter les tanks
spawnSystem->enableMultipleSpawnTypes({
    "enemy_red", 
    "enemy_blue", 
    "enemy_tank"
});

// Après 120 secondes - CAUCHEMAR
spawnSystem->enableMultipleSpawnTypes({
    "enemy_red", 
    "enemy_blue", 
    "enemy_fast", 
    "enemy_tank"
});
```

---

### Exemple 4 : Vagues avec pause

```cpp
// Vague 1 : Rouges uniquement
spawnSystem->setCurrentEnemyType("enemy_red");
// ... attendre 20 secondes ...

// Pause (désactiver le spawn)
spawnSystem->enableMultipleSpawnTypes({}); // Liste vide = pas de spawn

// Vague 2 : Bleus uniquement
spawnSystem->setCurrentEnemyType("enemy_blue");
// ... attendre 20 secondes ...

// Pause
spawnSystem->enableMultipleSpawnTypes({});

// Vague 3 : Les deux ensemble
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
```

---

### Exemple 5 : Boss waves

```cpp
// Vague normale
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});

// BOSS TIME ! (spawn un seul tank géant)
spawnSystem->enableMultipleSpawnTypes({}); // Arrêter les autres
spawnSystem->spawnEnemyGroup(world, "enemy_tank");

// Retour à la normale
spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
```

---

## 🔧 Configuration dans enemies.json

Pour que le spawn parallèle fonctionne bien, ajuste les intervalles :

```json
{
  "id": "enemy_red",
  "spawn": {
    "spawnInterval": 6.0    // ← Spawn toutes les 6 secondes
  }
},
{
  "id": "enemy_blue",
  "spawn": {
    "spawnInterval": 8.0    // ← Spawn toutes les 8 secondes
  }
}
```

**Astuce** : Utilise des intervalles différents pour éviter que tous les ennemis spawnen en même temps !

---

## 🎮 Dans quel fichier modifier ?

### Pour le comportement de base

**`server/src/Game.cpp`** - Ligne ~40-50 :

```cpp
if (m_enemyConfigManager->loadFromFile("server/config/enemies.json")) {
    spawnSystem->setEnemyConfigManager(m_enemyConfigManager);
    
    // 👇 CHANGE ICI
    spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
    
    std::cout << "[Game] Enemy configurations loaded successfully" << std::endl;
}
```

### Pour une progression dynamique

Crée une méthode dans `Game.cpp` :

```cpp
// Ajoute dans Game.hpp
private:
    float m_gameTime = 0.0F;

// Ajoute dans Game.cpp
void Game::updateWaveProgression(float deltaTime)
{
    m_gameTime += deltaTime;
    
    if (m_gameTime < 30.0F) {
        // Phase 1: Rouges uniquement
        spawnSystem->setCurrentEnemyType("enemy_red");
    } else if (m_gameTime < 60.0F) {
        // Phase 2: Bleus uniquement
        spawnSystem->setCurrentEnemyType("enemy_blue");
    } else {
        // Phase 3: Les deux ensemble
        spawnSystem->enableMultipleSpawnTypes({"enemy_red", "enemy_blue"});
    }
}

// Appelle dans runGameLoop()
void Game::runGameLoop()
{
    // ... code existant ...
    
    updateWaveProgression(deltaTime);
    
    // ... reste du code ...
}
```

---

## 💡 Conseils

✅ **Spawn parallèle** : Idéal pour un gameplay intense  
✅ **Alternance** : Bon pour un rythme plus prévisible  
✅ **Manuel** : Parfait pour des vagues scriptées  

🎯 **Recommandation** : Utilise `enableMultipleSpawnTypes` avec 2-3 types d'ennemis max pour un bon équilibre !

---

## 🐛 Debugging

Pour voir ce qui spawn :

```cpp
// Dans SpawnSystem::update()
std::cout << "[Spawn] Spawning " << m_currentEnemyType << std::endl;
```

Ou dans `spawnEnemyFromConfig()` :

```cpp
std::cout << "[Spawn] Enemy " << enemyType 
          << " at (" << posX << ", " << posY << ")" << std::endl;
```
