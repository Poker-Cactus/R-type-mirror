# Fix: Player-Ally Collision Damage Issue

## Problem
When the player collides with the ally AI, the ally takes damage. This should not happen as allies are friendly entities that should not damage each other or the player through collision.

## Root Cause
In `DamageSystem.hpp`, the `handleCollision` function applies mutual damage when both colliding entities have health components:

```cpp
if (aHasHealth && bHasHealth) {
  // Both have health - if both are players, ignore collision damage
  if (aIsPlayer && bIsPlayer) {
    return; // Prevents player-player damage
  }
  // Otherwise apply mutual damage (PROBLEM: includes player-ally collisions)
  applyDamage(world, entityA, entityB, damageFromEntityCollision);
  applyDamage(world, entityB, entityA, damageFromEntityCollision);
}
```

The system only prevented damage between two players, but not between players and allies.

## Solution
Added a check to prevent damage when a player collides with an ally, similar to the existing player-player check:

```cpp
// Also prevent damage when player collides with ally
bool aIsAlly = world.hasComponent<ecs::Ally>(entityA);
bool bIsAlly = world.hasComponent<ecs::Ally>(entityB);

if ((aIsPlayer && bIsAlly) || (aIsAlly && bIsPlayer)) {
  // Do not apply damage when player collides with ally
  return;
}
```

## Files Modified
- `server/include/systems/DamageSystem.hpp`: Added player-ally collision prevention

## Build Status
✅ **Build successful** - No compilation errors

## Testing
The fix prevents damage application when:
- Player collides with ally AI
- Ally AI collides with player

While still allowing damage in legitimate cases:
- Player collides with enemy
- Enemy collides with enemy
- Projectile hits any entity (handled separately)

## Note
The `applyDamage` function already had friendly fire prevention for projectiles, but this was specifically for the entity-on-entity collision case where mutual damage was being applied.
