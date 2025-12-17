# Asset Directory Structure

This directory contains all game assets used by the R-Type client.

## Sprite Assets

Place your sprite texture files in `sprites/` subdirectory:

```
client/assets/sprites/
├── player_ship.png     # Player ship sprite (SpriteId::PLAYER_SHIP = 1)
├── enemy_ship.png      # Enemy ship sprite (SpriteId::ENEMY_SHIP = 2)
├── projectile.png      # Projectile sprite (SpriteId::PROJECTILE = 3)
├── explosion.png       # Explosion sprite (SpriteId::EXPLOSION = 4)
└── powerup.png         # Power-up sprite (SpriteId::POWERUP = 5)
```

### How It Works

1. **Server-Driven Visual Identity**: The server decides what each entity looks like by assigning a `Sprite` component with a `spriteId` value.

2. **Texture Loading**: When the game starts, `PlayingState::loadSpriteTextures()` loads all sprite textures into memory:
   ```cpp
   m_spriteTextures[SpriteId::PLAYER_SHIP] = renderer->loadTexture("client/assets/sprites/player_ship.png");
   m_spriteTextures[SpriteId::ENEMY_SHIP] = renderer->loadTexture("client/assets/sprites/enemy_ship.png");
   // etc...
   ```

3. **Rendering**: The client looks up the texture based on the `spriteId` received from the server:
   ```cpp
   auto textureIt = m_spriteTextures.find(sprite.spriteId);
   if (textureIt != m_spriteTextures.end() && textureIt->second != nullptr) {
     renderer->drawTextureEx(textureIt->second, x, y, width, height, 0.0, false, false);
   }
   ```

4. **Fallback**: If a texture is missing, the client falls back to colored rectangles:
   - Player ships: Blue
   - Enemy ships: Red
   - Projectiles: Yellow
   - Power-ups: Green
   - Explosions: Orange

### Asset Requirements

- **Format**: PNG (recommended), or any format supported by SDL2_image
- **Size**: Any size - sprites will be scaled to the dimensions specified in the `Sprite` component
- **Recommended Dimensions**: 
  - Player/Enemy ships: 64x64 or 128x128 pixels
  - Projectiles: 16x16 or 32x32 pixels
  - Power-ups: 32x32 or 64x64 pixels
  - Explosions: 64x64 or 128x128 pixels

### Architecture Notes

- **No Client-Side Inference**: The client never guesses what an entity should look like
- **Server Authority**: All visual identity is determined by the server via the `Sprite` component
- **Network Replication**: The `Sprite` component is replicated from server to client in entity snapshots
- **Pure Renderer**: The client is a pure renderer that maps `spriteId` → texture, nothing more

### Adding New Sprites

To add a new sprite type:

1. Add a new constant to `engineCore/include/ecs/components/Sprite.hpp`:
   ```cpp
   namespace SpriteId {
     constexpr uint32_t BOSS_SHIP = 6;
   }
   ```

2. Assign the sprite on the server when creating entities (e.g., in `SpawnSystem` or `Game.cpp`)

3. Add texture loading in `PlayingState::loadSpriteTextures()`:
   ```cpp
   m_spriteTextures[SpriteId::BOSS_SHIP] = 
     renderer->loadTexture("client/assets/sprites/boss_ship.png");
   ```

4. (Optional) Add fallback color in `PlayingState::render()`:
   ```cpp
   case SpriteId::BOSS_SHIP:
     color = {128, 0, 128, 255}; // Purple
     break;
   ```

5. Place your `boss_ship.png` file in `client/assets/sprites/`

That's it! No changes needed to network protocol or ECS systems - the sprite system is fully data-driven.
