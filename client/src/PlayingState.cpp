/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.cpp
*/

#include "PlayingState.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../network/include/AsioClient.hpp"
#include "../include/AssetPath.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/Geometry.hpp"
#include "../interface/KeyCodes.hpp"
#include <iostream>

PlayingState::PlayingState(std::shared_ptr<IRenderer> renderer, const std::shared_ptr<ecs::World> &world,
                           Settings &settings, std::shared_ptr<INetworkManager> networkManager)
    : renderer(std::move(renderer)), world(world), background(nullptr), settings(settings),
      m_networkManager(networkManager)
{
}

PlayingState::~PlayingState()
{
  cleanup();
}

bool PlayingState::init()
{
  if (renderer == nullptr) {
    std::cerr << "PlayingState: Renderer is null" << '\n';
    return false;
  }

  std::cout << "[PlayingState] Initializing with m_playerHealth = " << m_playerHealth << '\n';

  settingsMenu = std::make_shared<SettingsMenu>(renderer);

  // Initialiser le background parallaxe
  background = std::make_unique<ParallaxBackground>(renderer);
  if (!background->init()) {
    std::cerr << "PlayingState: Failed to initialize parallax background" << '\n';
    return false;
  }

  // Load sprite textures
  loadSpriteTextures();

  // Load hearts texture for health display
  try {
    m_heartsTexture = renderer->loadTexture("client/assets/life-bar/hearts.png");
    if (m_heartsTexture != nullptr) {
      std::cout << "[PlayingState] ✓ Loaded hearts.png for HP display" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load hearts.png" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load hearts.png: " << e.what() << '\n';
  }

  // Load HUD font with fallback
  try {
#ifdef __APPLE__
    // macOS system font path
    constexpr int HUD_FONT_SIZE = 18;
    m_hudFont = renderer->loadFont("/System/Library/Fonts/Helvetica.ttc", HUD_FONT_SIZE);
#else
    constexpr int HUD_FONT_SIZE = 18;
    m_hudFont = renderer->loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", HUD_FONT_SIZE);
#endif
  } catch (const std::exception &e) {
    std::cerr << "PlayingState: Warning - Could not load HUD font: " << e.what() << '\n';
    m_hudFont = nullptr;
  }

  // Initialize info mode
  m_infoMode = std::make_unique<InfoMode>(renderer, m_hudFont);

  std::cout << "PlayingState: Initialized successfully" << '\n';

  return true;
}

void PlayingState::update(float delta_time)
{
  // Calculate FPS
  m_fpsAccumulator += delta_time;
  m_fpsFrameCount++;

  // Update FPS every second
  if (m_fpsAccumulator >= 1.0f) {
    m_currentFps = static_cast<float>(m_fpsFrameCount) / m_fpsAccumulator;
    m_fpsAccumulator = 0.0f;
    m_fpsFrameCount = 0;
  }

  // Mettre à jour le background
  if (background) {
    background->update(delta_time);
  }
  changeAnimationPlayers(delta_time);

  // Update sprite animations
  updateAnimations(delta_time);

  // Update HUD data from world state
  updateHUDFromWorld(delta_time);

  // Update info mode
  if (m_infoMode) {
    m_infoMode->update(delta_time);
  }

  // Send ping periodically to measure latency
  if (m_networkManager) {
    m_pingTimer += delta_time;
    if (m_pingTimer >= 2.0f) { // Ping every 2 seconds
      static_cast<AsioClient *>(m_networkManager.get())->sendPing();
      m_pingTimer = 0.0f;
    }
  }
}

void PlayingState::render()
{
  // Dessiner le background en premier
  if (background) {
    background->render();
  }

  // CLIENT PURE RENDERER - NO GAMEPLAY INFERENCE
  // Visual identity is replicated data decided by the server.
  // The client is a pure renderer and must never infer game roles.

  if (world == nullptr || renderer == nullptr) {
    return;
  }

  // Render all entities that have Transform + Sprite
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Transform>());
  sig.set(ecs::getComponentId<ecs::Sprite>());

  std::vector<ecs::Entity> entities;
  world->getEntitiesWithSignature(sig, entities);

  for (auto entity : entities) {
    const auto &transformComponent = world->getComponent<ecs::Transform>(entity);
    const auto &sprite = world->getComponent<ecs::Sprite>(entity);

    // Try to use texture if available, otherwise fall back to colored rectangle
    auto textureIt = m_spriteTextures.find(sprite.spriteId);

    if (textureIt != m_spriteTextures.end() && textureIt->second != nullptr) {
      constexpr int PLAYER_FRAME_WIDTH = 33; // 166 / 5
      constexpr int PLAYER_FRAME_HEIGHT = 17; // 86 / 5

      bool rendered = false;

      // Check if sprite is animated
      if (sprite.animated && sprite.frameCount > 1) {
        // Calculate frame dimensions and source rectangle for animated sprites
        int frameWidth = 0;
        int frameHeight = 0;

        if (sprite.spriteId == ecs::SpriteId::ENEMY_SHIP) {
          // Enemy ship: 533x36 with 16 frames
          frameWidth = 533 / 16; // 33px per frame
          frameHeight = 36;
        } else if (sprite.spriteId == ecs::SpriteId::PLAYER_SHIP) {
          frameWidth = PLAYER_FRAME_WIDTH;
          frameHeight = PLAYER_FRAME_HEIGHT;
        } else if (sprite.spriteId == ecs::SpriteId::PROJECTILE) {
          frameWidth = 18;
          frameHeight = 14;
        } else if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
          // Yellow Bee: 256x64 with 2 rows x 8 columns = 16 frames
          frameWidth = 256 / 8; // 32px per frame
          frameHeight = 64 / 2; // 32px per frame (2 rows)
        } else if (sprite.spriteId == ecs::SpriteId::ENEMY_WALKER) {
          // Walker: 200x67 with 2 rows x 6 columns = 12 frames
          frameWidth = 200 / 6; // 33px per frame
          frameHeight = 67 / 2; // 33px per frame (2 rows)
        } else if (sprite.spriteId == ecs::SpriteId::WALKER_PROJECTILE) {
          // Walker Projectile: 549x72 with 7 frames in single row
          frameWidth = 549 / 7; // 78px per frame
          frameHeight = 72;
        } else if (sprite.spriteId == ecs::SpriteId::ENEMY_ROBOT) {
          // Robot: 200x34 with 6 frames in single row (0-2: left, 3-5: right)
          frameWidth = 200 / 6; // 33px per frame
          frameHeight = 34;
        } else if (sprite.spriteId == ecs::SpriteId::ROBOT_PROJECTILE) {
          // Robot Projectile: 101x114 single frame
          frameWidth = 101;
          frameHeight = 114;
        }

        if (frameWidth > 0 && frameHeight > 0) {
          // Calculate source rectangle based on current frame
          int srcX = 0;
          int srcY = 0;

          // For sprites with multiple rows (like Yellow Bee: 2 rows x 8 columns)
          if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
            // Yellow Bee: 16 frames in 2 rows of 8
            int framesPerRow = 8;
            int row = sprite.currentFrame / framesPerRow;
            int col = sprite.currentFrame % framesPerRow;
            srcX = col * frameWidth;
            srcY = row * frameHeight;
          } else if (sprite.spriteId == ecs::SpriteId::ENEMY_WALKER) {
            // Walker: 12 frames in 2 rows of 6
            // Row 0: walking animation (frames 0-5)
            // Row 1: shooting animation (frames 6-11)
            int framesPerRow = 6;
            int row = sprite.currentFrame / framesPerRow;
            int col = sprite.currentFrame % framesPerRow;
            srcX = col * frameWidth;
            srcY = row * frameHeight;
          } else if (sprite.spriteId == ecs::SpriteId::WALKER_PROJECTILE) {
            // Walker Projectile: single row spritesheet
            srcX = sprite.currentFrame * frameWidth;
            srcY = 0;
          } else if (sprite.spriteId == ecs::SpriteId::ENEMY_ROBOT) {
            // Robot: single row spritesheet with 6 frames
            srcX = sprite.currentFrame * frameWidth;
            srcY = 0;
          } else if (sprite.spriteId == ecs::SpriteId::ROBOT_PROJECTILE) {
            // Robot Projectile: single frame
            srcX = 0;
            srcY = 0;
          } else {
            // Single row spritesheets (ENEMY_SHIP, PLAYER_SHIP, PROJECTILE)
            srcX = sprite.currentFrame * frameWidth;
            srcY = 0;
          }

          // Apply transform scale to sprite dimensions
          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);

          // Debug: log animation state for enemy ships
          static float debugTimer = 0.0f;
          static float lastTime = 0.0f;
          if (sprite.spriteId == ecs::SpriteId::ENEMY_SHIP) {
            debugTimer += 0.016f; // Approximate frame time
            if (debugTimer - lastTime >= 1.0f) {
              std::cout << "[Render] Enemy sprite - animated: " << sprite.animated
                        << ", currentFrame: " << sprite.currentFrame << ", srcX: " << srcX
                        << ", frameWidth: " << frameWidth << std::endl;
              lastTime = debugTimer;
            }
          }

          renderer->drawTextureRegion(
            textureIt->second,
            {.x = srcX, .y = srcY, .width = frameWidth, .height = frameHeight}, // Source: current frame
            {.x = static_cast<int>(transformComponent.x),
             .y = static_cast<int>(transformComponent.y),
             .width = scaledWidth,
             .height = scaledHeight}); // Destination with scale applied
          rendered = true;
        }
      }

      // Non-animated or fallback rendering
      if (!rendered) {
        // Draw using actual texture
        if (sprite.spriteId == ecs::SpriteId::PLAYER_SHIP) {
          // Player ship is a spritesheet with player animation
          int srcX = m_playerFrameIndex * PLAYER_FRAME_WIDTH;
          int srcY = 0; // première ligne seulement
          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);
          renderer->drawTextureRegion(
            textureIt->second, {.x = srcX, .y = srcY, .width = PLAYER_FRAME_WIDTH, .height = PLAYER_FRAME_HEIGHT},
            {.x = static_cast<int>(transformComponent.x),
             .y = static_cast<int>(transformComponent.y),
             .width = scaledWidth,
             .height = scaledHeight}); // Destination with scale
        } else if (sprite.spriteId == ecs::SpriteId::PROJECTILE) {
          // Projectile is a spritesheet: 422x92 with 2 frames
          constexpr int PROJECTILE_FRAME_WIDTH = 18;
          constexpr int PROJECTILE_FRAME_HEIGHT = 14;
          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);
          renderer->drawTextureRegion(
            textureIt->second,
            {.x = 0, .y = 0, .width = PROJECTILE_FRAME_WIDTH, .height = PROJECTILE_FRAME_HEIGHT}, // Source: first frame
            {.x = static_cast<int>(transformComponent.x),
             .y = static_cast<int>(transformComponent.y),
             .width = scaledWidth,
             .height = scaledHeight}); // Destination with scale
        } else if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
          // Yellow Bee: Use frame 8 (first of bottom row = left-facing)
          constexpr int YELLOW_BEE_FRAME_WIDTH = 32;
          constexpr int YELLOW_BEE_FRAME_HEIGHT = 32;
          constexpr int YELLOW_BEE_FRAME = 8; // Bottom row, first frame
          int framesPerRow = 8;
          int row = YELLOW_BEE_FRAME / framesPerRow;
          int col = YELLOW_BEE_FRAME % framesPerRow;
          int srcX = col * YELLOW_BEE_FRAME_WIDTH;
          int srcY = row * YELLOW_BEE_FRAME_HEIGHT;

          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);

          // Calculate rotation angle based on velocity
          float rotation = transformComponent.rotation;

          renderer->drawTextureRegionEx(
            textureIt->second,
            {.x = srcX, .y = srcY, .width = YELLOW_BEE_FRAME_WIDTH, .height = YELLOW_BEE_FRAME_HEIGHT},
            {.x = static_cast<int>(transformComponent.x),
             .y = static_cast<int>(transformComponent.y),
             .width = scaledWidth,
             .height = scaledHeight},
            rotation, false, false);
        } else {
          // Other sprites: draw full texture
          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);
          renderer->drawTextureEx(textureIt->second, static_cast<int>(transformComponent.x),
                                  static_cast<int>(transformComponent.y), scaledWidth, scaledHeight, 0.0, false, false);
        }
      }
    } else {
      // Fallback: colored rectangles for missing textures
      constexpr Color COLOR_WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
      constexpr Color COLOR_PLAYER_BLUE = {.r = 100, .g = 150, .b = 255, .a = 255};
      constexpr Color COLOR_ENEMY_RED = {.r = 255, .g = 100, .b = 100, .a = 255};
      constexpr Color COLOR_ENEMY_YELLOW = {.r = 255, .g = 255, .b = 50, .a = 255};
      constexpr Color COLOR_PROJECTILE_YELLOW = {.r = 255, .g = 255, .b = 100, .a = 255};
      constexpr Color COLOR_POWERUP_GREEN = {.r = 100, .g = 255, .b = 100, .a = 255};
      constexpr Color COLOR_EXPLOSION_ORANGE = {.r = 255, .g = 150, .b = 50, .a = 255};
      constexpr Color COLOR_FALLBACK_GRAY = {.r = 200, .g = 200, .b = 200, .a = 255};

      Color color = COLOR_WHITE;

      switch (sprite.spriteId) {
      case ecs::SpriteId::PLAYER_SHIP:
        color = COLOR_PLAYER_BLUE;
        break;
      case ecs::SpriteId::ENEMY_SHIP:
        color = COLOR_ENEMY_RED;
        break;
      case ecs::SpriteId::ENEMY_YELLOW:
        color = COLOR_ENEMY_YELLOW;
        break;
      case ecs::SpriteId::ENEMY_WALKER:
        color = COLOR_ENEMY_RED;
        break;
      case ecs::SpriteId::WALKER_PROJECTILE:
        color = COLOR_PROJECTILE_YELLOW;
        break;
      case ecs::SpriteId::ENEMY_ROBOT:
        color = COLOR_ENEMY_YELLOW;
        break;
      case ecs::SpriteId::ROBOT_PROJECTILE:
        color = COLOR_PROJECTILE_YELLOW;
        break;
      case ecs::SpriteId::PROJECTILE:
        color = COLOR_PROJECTILE_YELLOW;
        break;
      case ecs::SpriteId::POWERUP:
        color = COLOR_POWERUP_GREEN;
        break;
      case ecs::SpriteId::EXPLOSION:
        color = COLOR_EXPLOSION_ORANGE;
        break;
      default:
        color = COLOR_FALLBACK_GRAY;
        break;
      }

      int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
      int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);
      renderer->drawRect(static_cast<int>(transformComponent.x), static_cast<int>(transformComponent.y), scaledWidth,
                         scaledHeight, color);
    }
  }

  // Draw HUD on top of everything
  renderHUD();
}

void PlayingState::renderHUD()
{
  if (renderer == nullptr) {
    return;
  }

  // Hearts texture properties
  constexpr int HEARTS_TEXTURE_WIDTH = 33;
  constexpr float HEART_ROW_HEIGHT = 76.0f / 7.0f; // 11.0 pixels per row, using float for precision
  constexpr int HEARTS_X = 20;
  constexpr int HEARTS_Y = 20;
  constexpr int DISPLAY_SCALE = 2; // Scale up for better visibility
  constexpr Color HUD_TEXT_WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
  constexpr int HUD_SCORE_OFFSET_Y = 50;

  // Draw hearts if texture is loaded
  if (m_heartsTexture != nullptr) {
    // Calculate heart display based on actual HP value
    // Each 100 HP = 1 full heart
    // Use floating point for precise heart calculation
    float heartsValue = static_cast<float>(m_playerHealth) / 100.0f;

    // Clamp to valid range (0.0 to 3.0 hearts max)
    heartsValue = std::max(0.0f, std::min(3.0f, heartsValue));

    // Convert hearts value to row index (0-6)
    // 3.0 hearts = row 0 (full)
    // 2.5 hearts = row 1
    // 2.0 hearts = row 2
    // 1.5 hearts = row 3
    // 1.0 hearts = row 4
    // 0.5 hearts = row 5
    // 0.0 hearts = row 6 (empty)

    int heartRow = 0;
    if (heartsValue >= 2.5f) {
      heartRow = 0; // 2.5-3.0 hearts: full
    } else if (heartsValue >= 2.0f) {
      heartRow = 1; // 2.0-2.4 hearts
    } else if (heartsValue >= 1.5f) {
      heartRow = 2; // 1.5-1.9 hearts
    } else if (heartsValue >= 1.0f) {
      heartRow = 3; // 1.0-1.4 hearts
    } else if (heartsValue >= 0.5f) {
      heartRow = 4; // 0.5-0.9 hearts
    } else if (heartsValue > 0.0f) {
      heartRow = 5; // 0.1-0.4 hearts
    } else {
      heartRow = 6; // 0 hearts: empty
    }

    // Calculate source Y position with rounding for exact pixel alignment
    int sourceY = static_cast<int>(std::round(heartRow * HEART_ROW_HEIGHT));

    // Draw the appropriate heart row
    renderer->drawTextureRegion(
      m_heartsTexture,
      {.x = 0, .y = sourceY, .width = HEARTS_TEXTURE_WIDTH, .height = static_cast<int>(std::round(HEART_ROW_HEIGHT))},
      {.x = HEARTS_X,
       .y = HEARTS_Y,
       .width = HEARTS_TEXTURE_WIDTH * DISPLAY_SCALE,
       .height = static_cast<int>(std::round(HEART_ROW_HEIGHT)) * DISPLAY_SCALE});
  }

  // Score text (only if font is loaded)
  if (m_hudFont != nullptr) {
    std::string scoreText = "Score: " + std::to_string(m_playerScore);
    renderer->drawText(m_hudFont, scoreText, HEARTS_X, HEARTS_Y + HUD_SCORE_OFFSET_Y, HUD_TEXT_WHITE);
  }

  // Render info mode if active
  if (m_infoMode) {
    const int infoTextY = HEARTS_Y + HUD_SCORE_OFFSET_Y + 30; // Below score
    m_infoMode->render(HEARTS_X, infoTextY);
  }
}

void PlayingState::updateAnimations(float deltaTime)
{
  if (world == nullptr) {
    return;
  }

  // Get all entities with sprite component
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Sprite>());

  std::vector<ecs::Entity> entities;
  world->getEntitiesWithSignature(sig, entities);

  int animatedCount = 0;
  for (auto entity : entities) {
    auto &sprite = world->getComponent<ecs::Sprite>(entity);

    if (!sprite.animated || sprite.frameCount <= 1) {
      continue;
    }

    // If the animation is non-looping and already finished, keep final frame
    if (!sprite.loop) {
      if (!sprite.reverseAnimation && sprite.currentFrame >= sprite.endFrame) {
        continue;
      }
      if (sprite.reverseAnimation && sprite.currentFrame <= sprite.endFrame) {
        continue;
      }
    }

    animatedCount++;

    // Update animation timer
    sprite.animationTimer += deltaTime;

    // Check if we should advance to next frame
    if (sprite.animationTimer >= sprite.frameTime) {
      sprite.animationTimer -= sprite.frameTime;

      if (sprite.reverseAnimation) {
        // Play animation in reverse (e.g., from frame 7 to 0)
        if (sprite.currentFrame > sprite.endFrame) {
          sprite.currentFrame--;
        } else if (sprite.loop) {
          sprite.currentFrame = sprite.startFrame; // Loop back
        }
      } else {
        // Play animation forward
        if (sprite.currentFrame < sprite.endFrame) {
          sprite.currentFrame++;
        } else if (sprite.loop) {
          sprite.currentFrame = sprite.startFrame; // Loop back
        }
      }
    }
  }

  // Debug: log once per second
  static float debugTimer = 0.0f;
  debugTimer += deltaTime;
  if (debugTimer >= 1.0f) {
    if (animatedCount > 0) {
      std::cout << "[Animation] " << animatedCount << " animated sprites active" << std::endl;
    }
    debugTimer = 0.0f;
  }
}

void PlayingState::updateHUDFromWorld(float deltaTime)
{
  if (world == nullptr) {
    return;
  }

  // Get our assigned client id from the NetworkSendSystem
  auto *sendSys = world->getSystem<NetworkSendSystem>();
  if (sendSys == nullptr) {
    return;
  }

  const std::uint32_t myClientId = sendSys->getClientId();

  // Find the entity whose Networked.networkId == myClientId
  ecs::ComponentSignature playerSig;
  playerSig.set(ecs::getComponentId<ecs::Networked>());
  playerSig.set(ecs::getComponentId<ecs::Health>());
  playerSig.set(ecs::getComponentId<ecs::Score>());

  std::vector<ecs::Entity> entities;
  world->getEntitiesWithSignature(playerSig, entities);

  // Debug: log entity count periodically
  constexpr int DEBUG_LOG_INTERVAL = 120;
  static int debug_counter = 0;
  if (++debug_counter % DEBUG_LOG_INTERVAL == 0) {
    std::cout << "[PlayingState] Candidate entities=" << entities.size() << " myClientId=" << myClientId
              << " m_playerHealth=" << m_playerHealth << '\n';
  }

  // Prefer finding entity by PlayerId.owner_client (explicit from server)
  bool found = false;
  for (auto entity : entities) {
    if (world->hasComponent<ecs::PlayerId>(entity)) {
      const auto &pid = world->getComponent<ecs::PlayerId>(entity);
      if (pid.clientId == myClientId) {
        if (world->hasComponent<ecs::Health>(entity)) {
          const auto &health = world->getComponent<ecs::Health>(entity);
          m_playerHealth = health.hp;
          m_playerMaxHealth = health.maxHp;
        }
        if (world->hasComponent<ecs::Score>(entity)) {
          const auto &score = world->getComponent<ecs::Score>(entity);
          m_playerScore = score.points;
        }
        found = true;
        break;
      }
    }
  }

  // Fallback: match by Networked.networkId == myClientId (legacy behavior)
  if (!found) {
    for (auto entity : entities) {
      if (!world->hasComponent<ecs::Networked>(entity)) {
        continue;
      }
      const auto &net = world->getComponent<ecs::Networked>(entity);
      if (static_cast<std::uint32_t>(net.networkId) != myClientId) {
        continue;
      }
      if (world->hasComponent<ecs::Health>(entity)) {
        const auto &health = world->getComponent<ecs::Health>(entity);
        m_playerHealth = health.hp;
      }
      if (world->hasComponent<ecs::Score>(entity)) {
        const auto &score = world->getComponent<ecs::Score>(entity);
        m_playerScore = score.points;
      }
      break; // found our player
    }
  }

  // Update info mode with current game data
  if (m_infoMode) {
    // Pass real FPS value
    m_infoMode->setGameData(m_playerHealth, m_playerScore, m_currentFps);

    // Collect real entity statistics
    int totalEntities = 0;
    int playerCount = 0;
    int enemyCount = 0;
    int projectileCount = 0;

    // Count all entities with Transform component (basic entities)
    {
      ecs::ComponentSignature allEntitiesSig;
      allEntitiesSig.set(ecs::getComponentId<ecs::Transform>());
      std::vector<ecs::Entity> allEntities;
      world->getEntitiesWithSignature(allEntitiesSig, allEntities);
      totalEntities = static_cast<int>(allEntities.size());
    }

    // Count players (entities with PlayerId component)
    {
      ecs::ComponentSignature playerSig;
      playerSig.set(ecs::getComponentId<ecs::PlayerId>());
      std::vector<ecs::Entity> players;
      world->getEntitiesWithSignature(playerSig, players);
      playerCount = static_cast<int>(players.size());
    }

    // Count enemies (entities with Pattern component - they have movement patterns)
    {
      ecs::ComponentSignature enemySig;
      enemySig.set(ecs::getComponentId<ecs::Pattern>());
      enemySig.set(ecs::getComponentId<ecs::Health>()); // Enemies typically have health
      std::vector<ecs::Entity> enemies;
      world->getEntitiesWithSignature(enemySig, enemies);
      enemyCount = static_cast<int>(enemies.size());
    }

    // Count projectiles (entities with Velocity but no Pattern - projectiles move linearly)
    {
      ecs::ComponentSignature projectileSig;
      projectileSig.set(ecs::getComponentId<ecs::Velocity>());
      projectileSig.set(ecs::getComponentId<ecs::Transform>());
      // Exclude entities with Pattern (enemies) or PlayerId (players)
      std::vector<ecs::Entity> projectiles;
      world->getEntitiesWithSignature(projectileSig, projectiles);

      // Filter out entities that have Pattern or PlayerId
      for (auto entity : projectiles) {
        if (!world->hasComponent<ecs::Pattern>(entity) && !world->hasComponent<ecs::PlayerId>(entity)) {
          projectileCount++;
        }
      }
    }

    // Calculate game time (using a simple accumulator for now)
    static float gameTimeAccumulator = 0.0f;
    gameTimeAccumulator += deltaTime;

    // Update game statistics in info mode
    m_infoMode->setGameStats(totalEntities, playerCount, enemyCount, projectileCount, gameTimeAccumulator);

    // Set real network data
    if (m_networkManager) {
      float latency = m_networkManager->getLatency();
      bool connected = m_networkManager->isConnected();
      int packetsPerSec = m_networkManager->getPacketsPerSecond();
      int uploadBps = m_networkManager->getUploadBytesPerSecond();
      int downloadBps = m_networkManager->getDownloadBytesPerSecond();
      m_infoMode->setNetworkData(latency, connected, packetsPerSec);
      m_infoMode->setNetworkBandwidth(uploadBps, downloadBps);
    } else {
      m_infoMode->setNetworkData(-1.0f, false, 0);
      m_infoMode->setNetworkBandwidth(0, 0);
    }
  }
}

void PlayingState::processInput()
{
  if (renderer == nullptr)
    return;

  if (renderer->isKeyPressed(settings.up)) {
    m_returnUp = true;
  } else if (renderer->isKeyPressed(settings.down)) {
    m_returnDown = true;
  } else {
    m_returnUp = false;
    m_returnDown = false;
  }

  // Handle info mode input
  if (m_infoMode) {
    m_infoMode->processInput();
  }
}

void PlayingState::changeAnimationPlayers(float delta_time)
{
  // No input: reset to idle and clear any queued single-shot animation
  if (!m_returnUp && !m_returnDown) {
    m_playerAnimTimer = 0.f;
    m_playerFrameIndex = 2;
    m_playerAnimDirection = PlayerAnimDirection::None;
    m_playerAnimPlayingOnce = false;
    m_playerAnimPhase = 0;
    return;
  }

  // Determine desired direction based on current input
  const PlayerAnimDirection desiredDirection = m_returnUp ? PlayerAnimDirection::Up : PlayerAnimDirection::Down;

  // Start a new single-shot animation only on a fresh key press or direction change
  if (desiredDirection != m_playerAnimDirection) {
    m_playerAnimDirection = desiredDirection;
    m_playerAnimPlayingOnce = true;
    m_playerAnimPhase = 0;
    m_playerAnimTimer = 0.f;
    m_playerFrameIndex = 2; // start from neutral frame before stepping
  }

  if (!m_playerAnimPlayingOnce) {
    return; // already played for this press
  }

  m_playerAnimTimer += delta_time;
  constexpr float ANIM_FRAME_DURATION = 0.12f; // slightly faster for snappier feel
  if (m_playerAnimTimer >= ANIM_FRAME_DURATION) {
    m_playerAnimTimer = 0.f;
    m_playerAnimPhase++;

    if (m_playerAnimDirection == PlayerAnimDirection::Up) {
      if (m_playerAnimPhase == 1) {
        m_playerFrameIndex = 3; // mid frame
      } else {
        m_playerFrameIndex = 4; // end frame
        m_playerAnimPlayingOnce = false; // done for this press
      }
    } else if (m_playerAnimDirection == PlayerAnimDirection::Down) {
      if (m_playerAnimPhase == 1) {
        m_playerFrameIndex = 1; // mid frame
      } else {
        m_playerFrameIndex = 0; // end frame
        m_playerAnimPlayingOnce = false; // done for this press
      }
    }
  }
}

void PlayingState::cleanup()
{
  if (background) {
    background->cleanup();
    background.reset();
  }

  // Free all loaded sprite textures
  freeSpriteTextures();

  // Free hearts texture
  if (m_heartsTexture != nullptr && renderer != nullptr) {
    renderer->freeTexture(m_heartsTexture);
    m_heartsTexture = nullptr;
  }

  // Free HUD font
  if (m_hudFont != nullptr && renderer != nullptr) {
    renderer->freeFont(m_hudFont);
    m_hudFont = nullptr;
  }

  std::cout << "PlayingState: Cleaned up" << '\n';
}

void PlayingState::loadSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  std::cout << "[PlayingState] Loading sprite textures..." << '\n';

  // Load each texture individually with error handling
  // PLAYER_SHIP = 1 (spritesheet: 2450x150, 7 frames, using first frame only)
  try {
    void *player_tex = renderer->loadTexture("client/assets/sprites/player_ship.gif");
    if (player_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::PLAYER_SHIP] = player_tex;
      std::cout << "[PlayingState] ✓ Loaded player_ship.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load player_ship.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load player_ship.png: " << e.what() << '\n';
  }

  // ENEMY_SHIP = 2 (animated spritesheet: 533x36, 16 frames)
  try {
    void *enemy_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/enemy_ship.gif").c_str());
    if (enemy_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ENEMY_SHIP] = enemy_tex;
      std::cout << "[PlayingState] ✓ Loaded enemy_ship.gif" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.gif (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.gif: " << e.what() << '\n';
  }

  // PROJECTILE = 3 (spritesheet: 422x92, 2 frames, using first frame only)
  try {
    void *projectile_tex = renderer->loadTexture("client/assets/sprites/r-typesheet1.png");
    if (projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::PROJECTILE] = projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded projectile.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load projectile.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load projectile.png: " << e.what() << '\n';
  }

  // POWERUP = 5
  try {
    void *powerup_tex = renderer->loadTexture("client/assets/sprites/powerup.png");
    if (powerup_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::POWERUP] = powerup_tex;
      std::cout << "[PlayingState] ✓ Loaded powerup.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load powerup.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load powerup.png: " << e.what() << '\n';
  }

  // EXPLOSION = 4
  try {
    void *explosion_tex = renderer->loadTexture("client/assets/sprites/explosion.png");
    if (explosion_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::EXPLOSION] = explosion_tex;
      std::cout << "[PlayingState] ✓ Loaded explosion.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load explosion.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load explosion.png: " << e.what() << '\n';
  }

  // ENEMY_YELLOW = 6 (animated spritesheet: 256x64, 2 rows x 8 columns = 16 frames)
  try {
    void *enemy_yellow_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/enemy_yellow.gif"));
    if (enemy_yellow_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ENEMY_YELLOW] = enemy_yellow_tex;
      std::cout << "[PlayingState] ✓ Loaded enemy_yellow.gif" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load enemy_yellow.gif (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load enemy_yellow.gif: " << e.what() << '\n';
  }

  // ENEMY_WALKER = 7 (animated spritesheet: 200x67, 2 rows x 6 columns = 12 frames)
  try {
    void *enemy_walker_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/walk_enemy.gif"));
    if (enemy_walker_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ENEMY_WALKER] = enemy_walker_tex;
      std::cout << "[PlayingState] ✓ Loaded walk_enemy.gif" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load walk_enemy.gif (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load walk_enemy.gif: " << e.what() << '\n';
  }

  // WALKER_PROJECTILE = 8 (animated spritesheet: 549x72, 7 frames)
  try {
    void *walker_projectile_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/walk_projectile.png"));
    if (walker_projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::WALKER_PROJECTILE] = walker_projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded walk_projectile.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load walk_projectile.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load walk_projectile.png: " << e.what() << '\n';
  }

  // ENEMY_ROBOT = 9 (animated spritesheet: 200x34, 6 frames in single row)
  try {
    void *enemy_robot_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/enemy_robot.gif"));
    if (enemy_robot_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ENEMY_ROBOT] = enemy_robot_tex;
      std::cout << "[PlayingState] ✓ Loaded enemy_robot.gif" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load enemy_robot.gif (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load enemy_robot.gif: " << e.what() << '\n';
  }
  // ROBOT_PROJECTILE = 10 (single frame: 101x114)
  try {
    void *robot_projectile_tex = renderer->loadTexture(resolveAssetPath("client/assets/sprites/robot_projectile.png"));
    if (robot_projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ROBOT_PROJECTILE] = robot_projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded robot_projectile.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load robot_projectile.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load robot_projectile.png: " << e.what() << '\n';
  }
  constexpr int EXPECTED_TEXTURE_COUNT = 10;
  std::cout << "[PlayingState] Successfully loaded " << m_spriteTextures.size() << " / " << EXPECTED_TEXTURE_COUNT
            << " sprite textures" << '\n';
  if (m_spriteTextures.size() < EXPECTED_TEXTURE_COUNT) {
    std::cerr << "[PlayingState] Missing textures will use fallback colored rectangles" << '\n';
  }
}

void PlayingState::freeSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  for (auto &[spriteId, texture] : m_spriteTextures) {
    if (texture != nullptr) {
      renderer->freeTexture(texture);
    }
  }

  m_spriteTextures.clear();
}
