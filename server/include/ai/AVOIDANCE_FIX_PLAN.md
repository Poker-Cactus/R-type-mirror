# Fix: Ally Collision Avoidance Logic

## Problem
The ally AI frequently creates collisions with enemies instead of avoiding them.
Reasons identified:
1. **Coordinate Mismatch**: The avoidance logic uses `Transform.x/y` (top-left) as the entity center, but actual collision uses the full bounding box.
2. **Hitbox Shape**: AI and enemies are rectangular, not points. Using top-left for distance checks creates a significant offset (up to half the sprite size).
3. **Prediction Offset**: Predicting position based on top-left without centering means the predicted "danger zone" is shifted relative to the actual danger zone.

## Solution
We will implement "Center-Based Avoidance":
1. Update `AllyAIUtility` to include a helper `getColliderCenter` that calculates `(x + w/2, y + h/2)`.
2. Refactor `AllyPerception` to use center coordinates for ALL distance and direction calculations.
3. Use the centers for:
   - Ally position (for self-preservation)
   - Enemy position (target tracking)
   - Projectile position (threat avoidance)
   - Center-screen direction calculation

## Plan
1.  Modify `AllyAIUtility.hpp/cpp`: Add `getColliderCenter`.
2.  Modify `AllyPerception.cpp`:
    - In `update`, get ally center.
    - In `evaluateEnemyThreats` and `evaluateProjectileThreats`, calculate threat centers.
    - Pass centers to `calculateAvoidanceDirection`.
    - Pass center to `applyCenterPreference`.

This ensures the AI perceives threats relative to its physical center, matching the user's intuition and the visual reality.
