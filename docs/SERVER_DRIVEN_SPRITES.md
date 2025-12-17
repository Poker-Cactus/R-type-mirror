# Server-Driven Sprite System Implementation

## Overview
This document describes the implementation of a server-authoritative sprite system that follows capability-based ECS architecture principles.

## Design Principle
**"Visual identity is replicated data decided by the server. The client is a pure renderer and must never infer game roles."**

## Architecture

### Core Component: `ecs::Sprite`
Location: `engineCore/include/ecs/components/Sprite.hpp`

```cpp
struct Sprite : public IComponent {
  std::uint32_t spriteId = 0;  // Abstract visual identifier
  std::uint32_t width = 32;
  std::uint32_t height = 32;
  
  nlohmann::json toJson() const override;
  static Sprite fromJson(const nlohmann::json &j);
};
```

### Sprite Identifiers
Abstract identifiers used by the server to assign visual identity:

```cpp
namespace ecs::SpriteId {
  constexpr std::uint32_t PLAYER_SHIP = 1;
  constexpr std::uint32_t ENEMY_SHIP = 2;
  constexpr std::uint32_t PROJECTILE = 3;
  constexpr std::uint32_t EXPLOSION = 4;
  constexpr std::uint32_t POWERUP = 5;
}
```

## Server-Side Implementation

### 1. Sprite Assignment at Entity Creation

**SpawnSystem** (`server/include/systems/SpawnSystem.hpp`):
- Enemies: Assigned `SpriteId::ENEMY_SHIP` during `spawnEnemy()`
- Projectiles: Assigned `SpriteId::PROJECTILE` during `spawnProjectile()`

**Player Creation**:
- `server/src/Game.cpp`: `spawnPlayer()` functions
- `network/src/AsioServer.cpp`: `createPlayerEntity()`
- All players assigned `SpriteId::PLAYER_SHIP`

**Key Point**: Sprite is assigned as DATA at creation time, never inferred by systems later.

### 2. Network Replication

**NetworkSendSystem** (`server/src/systems/NetworkSendSystem.cpp`):
- Includes Sprite component in entity snapshots
- Only components that exist are serialized
- Uses `sprite.toJson()` for serialization

Example snapshot format:
```json
{
  "type": "snapshot",
  "entities": [
    {
      "id": 42,
      "transform": { "x": 120, "y": 300 },
      "collider": { "w": 32, "h": 32 },
      "sprite": { "spriteId": 1, "width": 32, "height": 32 }
    }
  ]
}
```

## Client-Side Implementation

### 1. Sprite Deserialization

**ClientNetworkReceiveSystem** (`client/src/systems/NetworkReceiveSystem.cpp`):
- On `snapshot` message: applies Sprite updates from server
- Uses `Sprite::fromJson()` for deserialization
- Never infers sprite from other components like PlayerControlled or EnemyAI

```cpp
if (entityJson.contains("sprite") && entityJson["sprite"].is_object()) {
  const auto &spriteJson = entityJson["sprite"];
  ecs::Sprite sprite = ecs::Sprite::fromJson(spriteJson);
  
  if (!world.hasComponent<ecs::Sprite>(entity)) {
    world.addComponent(entity, sprite);
  } else {
    world.getComponent<ecs::Sprite>(entity) = sprite;
  }
}
```

### 2. Pure Rendering

**PlayingState::render()** (`client/src/PlayingState.cpp`):
- Signature requires: `Transform + Sprite`
- Maps spriteId to visual representation
- NO checks for PlayerControlled, EnemyAI, or Projectile components
- NO gameplay logic in rendering

Current prototype implementation uses color mapping:
- `PLAYER_SHIP` → Blue
- `ENEMY_SHIP` → Red
- `PROJECTILE` → Yellow
- `POWERUP` → Green
- `EXPLOSION` → Orange

**Future Extension**: Replace color mapping with texture loading:
```cpp
// Future implementation
std::unordered_map<uint32_t, void*> spriteTextures;
spriteTextures[SpriteId::PLAYER_SHIP] = loadTexture("assets/player.png");
spriteTextures[SpriteId::ENEMY_SHIP] = loadTexture("assets/enemy.png");
// ...
```

## Benefits

### 1. Clean Separation of Concerns
- Server: Handles gameplay logic and assigns visual identity
- Client: Pure renderer, no gameplay decisions

### 2. No Identity Branching
- Systems don't check: `if (isPlayer)` or `if (hasComponent<PlayerControlled>())`
- Rendering is data-driven, not logic-driven

### 3. Network Efficiency
- Visual identity sent once on entity creation
- Updates only when sprite changes (rare)
- No redundant type information

### 4. Capability-Based ECS
- Follows "What can this entity do?" not "What kind is it?"
- Sprite is a capability component (visual representation)
- Decoupled from gameplay role components

### 5. Easy to Extend
- Add new sprite types: just define new `SpriteId` constant
- Client mapping is local (no protocol changes)
- Server assigns appropriate sprite based on entity purpose

## What This System Does NOT Do

❌ EntityKind enums  
❌ Client-side sprite inference  
❌ Identity checks in rendering (`if (isPlayer)`)  
❌ Hardcoded sprite logic based on component presence  
❌ Duplicate sprite decision logic on client  

## Testing

Build verification:
```bash
./build.sh
# ✅ BUILD SUCCESSFUL
```

Expected behavior:
1. Server creates entities and assigns Sprite components
2. NetworkSendSystem includes sprites in snapshots
3. Client receives sprites and stores them
4. Client renders using only Transform + Sprite data
5. Different entity types visually distinguishable (colors in prototype)

## Files Modified

### Created/Updated:
- `engineCore/include/ecs/components/Sprite.hpp` - Core Sprite component
- `server/include/systems/SpawnSystem.hpp` - Enemy/projectile sprite assignment
- `server/src/Game.cpp` - Player sprite assignment
- `network/src/AsioServer.cpp` - Player sprite assignment
- `server/src/systems/NetworkSendSystem.cpp` - Sprite replication
- `client/src/systems/NetworkReceiveSystem.cpp` - Sprite deserialization
- `client/src/PlayingState.cpp` - Sprite-based rendering

## Next Steps

1. **Texture Loading**: Replace color mapping with actual texture assets
2. **Animation**: Add sprite animation frame data
3. **Sprite Variants**: Support multiple visual variants per entity type
4. **Effect Sprites**: Implement explosion and powerup sprites
5. **Client Asset Manifest**: Map spriteIds to texture paths in config file
