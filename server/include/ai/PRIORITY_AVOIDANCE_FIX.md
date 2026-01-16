# Amélioration de la Priorité d'Évitement

## Problème
Quand plusieurs ennemis étaient trop proches, l'IA avait du mal à déterminer lequel éviter en priorité. Les vecteurs de menace s'annulaient parfois, créant des situations d'évitement inefficaces.

## Solution Implémentée
Système de priorité basé sur la proximité relative :

### 1. **Suivi de la Menace la Plus Proche**
- Ajout du champ `closestDistance` dans `AvoidanceState`
- Mise à jour en temps réel lors de l'évaluation des menaces

### 2. **Pondération Prioritaire**
- **Menaces principales** (dans les 25% de la distance la plus proche) : poids normal
- **Menaces secondaires** (ennemis) : poids réduit à 30%
- **Menaces secondaires** (projectiles) : poids réduit à 40% (légèrement plus important car plus dangereux)

### 3. **Logique de Calcul**
```cpp
// Pour chaque menace détectée :
state.closestDistance = min(state.closestDistance, distance);

// Calcul du poids de priorité :
if (distance > closestDistance * 1.25f) {
  priorityWeight = 0.3f; // Réduction pour menaces lointaines
} else {
  priorityWeight = 1.0f; // Poids normal pour menaces proches
}

finalWeight = baseWeight * priorityWeight;
```

## Avantages
- ✅ **Évitement focalisé** : L'IA se concentre sur la menace la plus immédiate
- ✅ **Situations complexes gérées** : Plusieurs ennemis proches ne créent plus de confusion
- ✅ **Performance préservée** : Calcul léger, pas d'algorithmes complexes
- ✅ **Comportement naturel** : Priorise logiquement les dangers immédiats

## Fichiers Modifiés
- `server/include/ai/AllyPerception.hpp` : Ajout de `closestDistance` dans `AvoidanceState`
- `server/src/ai/AllyPerception.cpp` : Logique de priorité dans `evaluateEnemyThreats` et `evaluateProjectileThreats`

## Résultat
L'IA évite maintenant efficacement même dans les situations les plus complexes avec plusieurs ennemis proches, en donnant la priorité à la menace la plus dangereuse.</content>
<parameter name="filePath">/Users/marco/code/epitech_tek3/R-type-mirror/server/include/ai/PRIORITY_AVOIDANCE_FIX.md
