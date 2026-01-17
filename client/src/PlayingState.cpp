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
#include <unordered_set>

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
    void *rawFont = renderer->loadFont("/System/Library/Fonts/Helvetica.ttc", HUD_FONT_SIZE);
#else
    constexpr int HUD_FONT_SIZE = 18;
    void *rawFont = renderer->loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", HUD_FONT_SIZE);
#endif
    // Wrap in shared_ptr with custom deleter
    auto fontDeleter = [r = renderer](void *font) {
      if (font && r)
        r->freeFont(font);
    };
    m_hudFont = std::shared_ptr<void>(rawFont, fontDeleter);
  } catch (const std::exception &e) {
    std::cerr << "PlayingState: Warning - Could not load HUD font: " << e.what() << '\n';
    m_hudFont = nullptr;
  }

  // Initialize info mode
  m_infoMode = std::make_unique<rtype::InfoMode>(renderer, m_hudFont, settings);

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
    m_infoMode->update();
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

        switch (sprite.spriteId) {
        case ecs::SpriteId::ENEMY_SHIP:
          frameWidth = 533 / 16; // 33px per frame
          frameHeight = 36;
          break;
        case ecs::SpriteId::PLAYER_SHIP:
          frameWidth = PLAYER_FRAME_WIDTH;
          frameHeight = PLAYER_FRAME_HEIGHT;
          break;
        case ecs::SpriteId::PROJECTILE:
          frameWidth = 18;
          frameHeight = 14;
          break;
        case ecs::SpriteId::POWERUP:
          frameWidth = 12; // 84 / 7 = 12px per frame
          frameHeight = 12;
          break;
        case ecs::SpriteId::ENEMY_YELLOW:
          // Yellow Bee: 256x64 with 2 rows x 8 columns = 16 frames
          frameWidth = 256 / 8; // 32px per frame
          frameHeight = 64 / 2; // 32px per frame (2 rows)
          break;
        case ecs::SpriteId::CHARGED_PROJECTILE:
          frameWidth = 165 / 2; // 82px per frame
          frameHeight = 16;
          break;
        case ecs::SpriteId::LOADING_SHOT:
          frameWidth = 255 / 8; // 31-32px per frame
          frameHeight = 29;
          break;
        case ecs::SpriteId::ENEMY_WALKER:
          // Walker: 200x67 with 2 rows x 6 columns = 12 frames
          frameWidth = 200 / 6; // 33px per frame
          frameHeight = 67 / 2; // 33px per frame (2 rows)
          break;
        case ecs::SpriteId::WALKER_PROJECTILE:
          // Walker Projectile: 549x72 with 7 frames in single row
          frameWidth = 549 / 7; // 78px per frame
          frameHeight = 72;
          break;
        case ecs::SpriteId::DRONE:
        case ecs::SpriteId::BUBBLE:
        case ecs::SpriteId::BUBBLE_TRIPLE:
        case ecs::SpriteId::BUBBLE_RUBAN1:
        case ecs::SpriteId::BUBBLE_RUBAN2:
        case ecs::SpriteId::BUBBLE_RUBAN3:
        // Individual bubble ruban animation frames
        case ecs::SpriteId::BUBBLE_RUBAN_BACK1:
        case ecs::SpriteId::BUBBLE_RUBAN_BACK2:
        case ecs::SpriteId::BUBBLE_RUBAN_BACK3:
        case ecs::SpriteId::BUBBLE_RUBAN_BACK4:
        case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1:
        case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE2:
        case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE3:
        case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE4:
        case ecs::SpriteId::BUBBLE_RUBAN_FRONT1:
        case ecs::SpriteId::BUBBLE_RUBAN_FRONT2:
        case ecs::SpriteId::BUBBLE_RUBAN_FRONT3:
        case ecs::SpriteId::BUBBLE_RUBAN_FRONT4:
        case ecs::SpriteId::TRIPLE_PROJECTILE:
        case ecs::SpriteId::TRIPLE_PROJECTILE_RIGHT:
        case ecs::SpriteId::TRIPLE_PROJECTILE_UP:
        case ecs::SpriteId::TRIPLE_PROJECTILE_DOWN:
        case ecs::SpriteId::RUBAN1_PROJECTILE:
        case ecs::SpriteId::RUBAN2_PROJECTILE:
        case ecs::SpriteId::RUBAN3_PROJECTILE:
        case ecs::SpriteId::RUBAN4_PROJECTILE:
        case ecs::SpriteId::RUBAN5_PROJECTILE:
        case ecs::SpriteId::RUBAN6_PROJECTILE:
        case ecs::SpriteId::RUBAN7_PROJECTILE:
        case ecs::SpriteId::RUBAN8_PROJECTILE:
        case ecs::SpriteId::RUBAN9_PROJECTILE:
        case ecs::SpriteId::RUBAN10_PROJECTILE:
        case ecs::SpriteId::RUBAN11_PROJECTILE:
        case ecs::SpriteId::RUBAN12_PROJECTILE:
        case ecs::SpriteId::RUBAN13_PROJECTILE:
        case ecs::SpriteId::RUBAN14_PROJECTILE:
          // Use sprite dimensions from server
          frameWidth = static_cast<int>(sprite.width);
          frameHeight = static_cast<int>(sprite.height);

          // Debug ruban projectiles
          if (sprite.spriteId >= ecs::SpriteId::RUBAN1_PROJECTILE &&
              sprite.spriteId <= ecs::SpriteId::RUBAN5_PROJECTILE) {
            static bool logged = false;
            if (!logged) {
              std::cout << "[Render] Ruban projectile - spriteId: " << sprite.spriteId << ", width: " << sprite.width
                        << ", height: " << sprite.height << ", frameCount: " << sprite.frameCount << std::endl;
              logged = true;
            }
          }
          break;
        case ecs::SpriteId::ENEMY_ROBOT:
          frameWidth = 200 / 6; // 33px per frame
          frameHeight = 34;
          break;
        case ecs::SpriteId::ROBOT_PROJECTILE:
          frameWidth = 101;
          frameHeight = 114;
          break;
        case ecs::SpriteId::DEATH_ANIM:
          frameWidth = 586 / 6; // 6 frames horizontally
          frameHeight = 94;
          break;
        default:
          break;
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
          } else if (sprite.spriteId == ecs::SpriteId::DEATH_ANIM) {
            // Death animation: horizontal spritesheet with 6 frames
            srcX = sprite.currentFrame * frameWidth;
            srcY = 0;
          } else if (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 &&
                     sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4) {
            // Individual bubble ruban frames: single image per file, no spritesheet
            srcX = 0;
            srcY = 0;
          } else if (sprite.spriteId >= ecs::SpriteId::TRIPLE_PROJECTILE_RIGHT &&
                     sprite.spriteId <= ecs::SpriteId::TRIPLE_PROJECTILE_DOWN) {
            // Triple projectile direction sprites: single image per file
            srcX = 0;
            srcY = 0;
          } else {
            // For Ruban and other sprites: use offsetX/offsetY/row if present
            srcX = static_cast<int>(sprite.offsetX) + (sprite.currentFrame * frameWidth);
            srcY = static_cast<int>(sprite.offsetY) + (sprite.row * frameHeight);
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
          int frameIndex = sprite.animated ? sprite.currentFrame : m_playerFrameIndex;
          int srcX = frameIndex * PLAYER_FRAME_WIDTH;
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
        } else if (sprite.spriteId == ecs::SpriteId::POWERUP) {
          // Powerup: R-Type_Items.png (84x12 total, 7 frames, using first 4)
          constexpr int POWERUP_FRAME_WIDTH = 12; // 84 / 7 = 12px per frame
          constexpr int POWERUP_FRAME_HEIGHT = 12;
          int srcX = sprite.currentFrame * POWERUP_FRAME_WIDTH;
          int scaledWidth = static_cast<int>(sprite.width * transformComponent.scale);
          int scaledHeight = static_cast<int>(sprite.height * transformComponent.scale);
          renderer->drawTextureRegion(textureIt->second,
                                      {.x = srcX, .y = 0, .width = POWERUP_FRAME_WIDTH, .height = POWERUP_FRAME_HEIGHT},
                                      {.x = static_cast<int>(transformComponent.x),
                                       .y = static_cast<int>(transformComponent.y),
                                       .width = scaledWidth,
                                       .height = scaledHeight});
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
      case ecs::SpriteId::BUBBLE:
      case ecs::SpriteId::BUBBLE_TRIPLE:
      case ecs::SpriteId::BUBBLE_RUBAN1:
      case ecs::SpriteId::BUBBLE_RUBAN2:
      case ecs::SpriteId::BUBBLE_RUBAN3:
      case ecs::SpriteId::DRONE:
        color = COLOR_POWERUP_GREEN; // Drones use same color as powerups
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
  if (m_hudFont) {
    std::string scoreText = "Score: " + std::to_string(m_playerScore);
    renderer->drawText(m_hudFont.get(), scoreText, HEARTS_X, HEARTS_Y + HUD_SCORE_OFFSET_Y, HUD_TEXT_WHITE);
  }

  // Render info mode if active
  if (m_infoMode) {
    m_infoMode->render();
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

void PlayingState::resetPlayerAnimation()
{
  m_returnUp = false;
  m_returnDown = false;
  m_playerAnimTimer = 0.f;
  m_playerFrameIndex = 2;
  m_playerAnimDirection = PlayerAnimDirection::None;
  m_playerAnimPlayingOnce = false;
  m_playerAnimPhase = 0;
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

  // Free HUD font (handled by shared_ptr destructor)
  m_hudFont.reset();

  std::cout << "PlayingState: Cleaned up" << '\n';
}

void PlayingState::setSoloMode(bool isSolo)
{
  m_isSolo = isSolo;
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
    void *projectile_tex = renderer->loadTexture("client/assets/sprites/simpleShot.png");
    if (projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::PROJECTILE] = projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded projectile.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load projectile.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load projectile.png: " << e.what() << '\n';
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

  // POWERUP = 5 (spritesheet: R-Type_Items.png with 7 frames, using first 4)
  try {
    void *powerup_tex = renderer->loadTexture("client/assets/R-Type_Items.png");
    if (powerup_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::POWERUP] = powerup_tex;
      std::cout << "[PlayingState] ✓ Loaded R-Type_Items.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load R-Type_Items.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load R-Type_Items.png: " << e.what() << '\n';
  }

  // DRONE = 6 (uses powerup texture as fallback)
  try {
    void *drone_tex = renderer->loadTexture("client/assets/r-typesheet3.gif");
    if (drone_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::DRONE] = drone_tex;
      std::cout << "[PlayingState] ✓ Loaded r-typesheet2.gif" << '\n';
    } else {
      // Fallback to powerup texture for drones
      if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
        m_spriteTextures[ecs::SpriteId::DRONE] = m_spriteTextures[ecs::SpriteId::POWERUP];
        std::cout << "[PlayingState] ✓ Using powerup.png for drone (fallback)" << '\n';
      }
    }
  } catch (const std::exception &e) {
    // Fallback to powerup texture for drones
    if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
      m_spriteTextures[ecs::SpriteId::DRONE] = m_spriteTextures[ecs::SpriteId::POWERUP];
      std::cout << "[PlayingState] ✓ Using powerup.png for drone (fallback after error)" << '\n';
    }
  }

  // BUBBLE = 7 (uses powerup texture as fallback)
  try {
    void *bubble_tex = renderer->loadTexture("client/assets/sprites/bubble.png");
    if (bubble_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::BUBBLE] = bubble_tex;
      std::cout << "[PlayingState] ✓ Loaded bubble.png" << '\n';
    } else {
      // Fallback to powerup texture for bubbles
      if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
        m_spriteTextures[ecs::SpriteId::BUBBLE] = m_spriteTextures[ecs::SpriteId::POWERUP];
        std::cout << "[PlayingState] ✓ Using powerup.png for bubble triple (fallback)" << '\n';
      }
    }
  } catch (const std::exception &e) {
    // Fallback to powerup texture for bubbles
    if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
      m_spriteTextures[ecs::SpriteId::BUBBLE] = m_spriteTextures[ecs::SpriteId::POWERUP];
      std::cout << "[PlayingState] ✓ Using powerup.png for bubble triple (fallback after error)" << '\n';
    }
  }

  // BUBBLE = 8 (uses powerup texture as fallback)
  try {
    void *buble_triple_tex = renderer->loadTexture("client/assets/sprites/bubble_triple.png");
    if (buble_triple_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::BUBBLE_TRIPLE] = buble_triple_tex;
      std::cout << "[PlayingState] ✓ Loaded bubble_triple.png" << '\n';
    } else {
      // Fallback to powerup texture for bubbles
      if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
        m_spriteTextures[ecs::SpriteId::BUBBLE_TRIPLE] = m_spriteTextures[ecs::SpriteId::POWERUP];
        std::cout << "[PlayingState] ✓ Using powerup.png for bubble triple (fallback)" << '\n';
      }
    }
  } catch (const std::exception &e) {
    // Fallback to powerup texture for BUBBLE_TRIPLEs
    if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
      m_spriteTextures[ecs::SpriteId::BUBBLE_TRIPLE] = m_spriteTextures[ecs::SpriteId::POWERUP];
      std::cout << "[PlayingState] ✓ Using powerup.png for bubble triple (fallback after error)" << '\n';
    }
  }

  // Load all 12 bubble ruban frames (4 back + 4 middle + 4 front)
  // BACK frames (compressed/backward)
  const std::array<std::uint32_t, 4> backSpriteIds = {
    ecs::SpriteId::BUBBLE_RUBAN_BACK1, ecs::SpriteId::BUBBLE_RUBAN_BACK2, ecs::SpriteId::BUBBLE_RUBAN_BACK3,
    ecs::SpriteId::BUBBLE_RUBAN_BACK4};
  for (int i = 0; i < 4; i++) {
    std::string path = "client/assets/sprites/bubble_ruban_sprite/bubble_ruban_back" + std::to_string(i + 1) + ".png";
    try {
      void *tex = renderer->loadTexture(path.c_str());
      if (tex != nullptr) {
        m_spriteTextures[backSpriteIds[i]] = tex;
        std::cout << "[PlayingState] ✓ Loaded bubble_ruban_back" << (i + 1) << ".png" << '\n';
      }
    } catch (const std::exception &e) {
      std::cerr << "[PlayingState] ✗ Failed to load " << path << ": " << e.what() << '\n';
    }
  }
  // Also set BUBBLE_RUBAN1 to point to first back frame for compatibility
  if (m_spriteTextures.find(ecs::SpriteId::BUBBLE_RUBAN_BACK1) != m_spriteTextures.end()) {
    m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN1] = m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN_BACK1];
  }

  // MIDDLE frames (neutral)
  const std::array<std::uint32_t, 4> middleSpriteIds = {
    ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1, ecs::SpriteId::BUBBLE_RUBAN_MIDDLE2, ecs::SpriteId::BUBBLE_RUBAN_MIDDLE3,
    ecs::SpriteId::BUBBLE_RUBAN_MIDDLE4};
  for (int i = 0; i < 4; i++) {
    std::string path = "client/assets/sprites/bubble_ruban_sprite/bubble_ruban_middle" + std::to_string(i + 1) + ".png";
    try {
      void *tex = renderer->loadTexture(path.c_str());
      if (tex != nullptr) {
        m_spriteTextures[middleSpriteIds[i]] = tex;
        std::cout << "[PlayingState] ✓ Loaded bubble_ruban_middle" << (i + 1) << ".png" << '\n';
      }
    } catch (const std::exception &e) {
      std::cerr << "[PlayingState] ✗ Failed to load " << path << ": " << e.what() << '\n';
    }
  }
  // Also set BUBBLE_RUBAN2 to point to first middle frame for compatibility
  if (m_spriteTextures.find(ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1) != m_spriteTextures.end()) {
    m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN2] = m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1];
  }

  // FRONT frames (stretched/forward)
  const std::array<std::uint32_t, 4> frontSpriteIds = {
    ecs::SpriteId::BUBBLE_RUBAN_FRONT1, ecs::SpriteId::BUBBLE_RUBAN_FRONT2, ecs::SpriteId::BUBBLE_RUBAN_FRONT3,
    ecs::SpriteId::BUBBLE_RUBAN_FRONT4};
  for (int i = 0; i < 4; i++) {
    std::string path = "client/assets/sprites/bubble_ruban_sprite/bubble_ruban_front" + std::to_string(i + 1) + ".png";
    try {
      void *tex = renderer->loadTexture(path.c_str());
      if (tex != nullptr) {
        m_spriteTextures[frontSpriteIds[i]] = tex;
        std::cout << "[PlayingState] ✓ Loaded bubble_ruban_front" << (i + 1) << ".png" << '\n';
      }
    } catch (const std::exception &e) {
      std::cerr << "[PlayingState] ✗ Failed to load " << path << ": " << e.what() << '\n';
    }
  }
  // Also set BUBBLE_RUBAN3 to point to first front frame for compatibility
  if (m_spriteTextures.find(ecs::SpriteId::BUBBLE_RUBAN_FRONT1) != m_spriteTextures.end()) {
    m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN3] = m_spriteTextures[ecs::SpriteId::BUBBLE_RUBAN_FRONT1];
  }

  // triple_projectile = 15 (uses bubble_shoot.png) - legacy, kept for compatibility
  try {
    void *triple_projectile_tex = renderer->loadTexture("client/assets/bubble_shoot.png");
    if (triple_projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE] = triple_projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded bubble_shoot.png for TRIPLE_PROJECTILE" << '\n';
    } else {
      // Fallback to projectile texture
      if (m_spriteTextures.find(ecs::SpriteId::PROJECTILE) != m_spriteTextures.end()) {
        m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE] = m_spriteTextures[ecs::SpriteId::PROJECTILE];
        std::cout << "[PlayingState] ✓ Using projectile texture for TRIPLE_PROJECTILE (fallback)" << '\n';
      }
    }
  } catch (const std::exception &e) {
    // Fallback to projectile texture for TRIPLE_PROJECTILE
    if (m_spriteTextures.find(ecs::SpriteId::PROJECTILE) != m_spriteTextures.end()) {
      m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE] = m_spriteTextures[ecs::SpriteId::PROJECTILE];
      std::cout << "[PlayingState] ✓ Using projectile texture for TRIPLE_PROJECTILE (fallback after error)" << '\n';
    }
  }

  // Triple projectile direction sprites
  try {
    void *triple_right_tex = renderer->loadTexture("client/assets/sprites/triple_projectile_srpite/triple_right.png");
    if (triple_right_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE_RIGHT] = triple_right_tex;
      std::cout << "[PlayingState] ✓ Loaded triple_right.png" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load triple_right.png: " << e.what() << '\n';
  }
  try {
    void *triple_up_tex = renderer->loadTexture("client/assets/sprites/triple_projectile_srpite/triple_up.png");
    if (triple_up_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE_UP] = triple_up_tex;
      std::cout << "[PlayingState] ✓ Loaded triple_up.png" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load triple_up.png: " << e.what() << '\n';
  }
  try {
    void *triple_down_tex = renderer->loadTexture("client/assets/sprites/triple_projectile_srpite/triple_down.png");
    if (triple_down_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::TRIPLE_PROJECTILE_DOWN] = triple_down_tex;
      std::cout << "[PlayingState] ✓ Loaded triple_down.png" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load triple_down.png: " << e.what() << '\n';
  }

  // Ruban projectile sprites (24 phases): Xruban_projectile.png where X = 1-24
  // Sprite IDs: RUBAN1_PROJECTILE (16) through RUBAN24_PROJECTILE (39)
  constexpr std::uint32_t RUBAN_SPRITE_IDS[] = {
    ecs::SpriteId::RUBAN1_PROJECTILE,  ecs::SpriteId::RUBAN2_PROJECTILE,  ecs::SpriteId::RUBAN3_PROJECTILE,
    ecs::SpriteId::RUBAN4_PROJECTILE,  ecs::SpriteId::RUBAN5_PROJECTILE,  ecs::SpriteId::RUBAN6_PROJECTILE,
    ecs::SpriteId::RUBAN7_PROJECTILE,  ecs::SpriteId::RUBAN8_PROJECTILE,  ecs::SpriteId::RUBAN9_PROJECTILE,
    ecs::SpriteId::RUBAN10_PROJECTILE, ecs::SpriteId::RUBAN11_PROJECTILE, ecs::SpriteId::RUBAN12_PROJECTILE,
    ecs::SpriteId::RUBAN13_PROJECTILE, ecs::SpriteId::RUBAN14_PROJECTILE, ecs::SpriteId::RUBAN15_PROJECTILE,
    ecs::SpriteId::RUBAN16_PROJECTILE, ecs::SpriteId::RUBAN17_PROJECTILE, ecs::SpriteId::RUBAN18_PROJECTILE,
    ecs::SpriteId::RUBAN19_PROJECTILE, ecs::SpriteId::RUBAN20_PROJECTILE, ecs::SpriteId::RUBAN21_PROJECTILE,
    ecs::SpriteId::RUBAN22_PROJECTILE, ecs::SpriteId::RUBAN23_PROJECTILE, ecs::SpriteId::RUBAN24_PROJECTILE};

  for (int i = 1; i <= 24; i++) {
    std::string texturePath =
      "client/assets/sprites/ruban_projectile_sprite/" + std::to_string(i) + "ruban_projectile.png";
    std::uint32_t spriteId = RUBAN_SPRITE_IDS[i - 1];
    try {
      void *ruban_tex = renderer->loadTexture(texturePath.c_str());
      if (ruban_tex != nullptr) {
        m_spriteTextures[spriteId] = ruban_tex;
        std::cout << "[PlayingState] ✓ Loaded " << i << "ruban_projectile.png" << '\n';
      } else {
        // Fallback to powerup texture
        if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
          m_spriteTextures[spriteId] = m_spriteTextures[ecs::SpriteId::POWERUP];
          std::cout << "[PlayingState] ✓ Using powerup.png for ruban" << i << " (fallback)" << '\n';
        }
      }
    } catch (const std::exception &e) {
      // Fallback to powerup texture
      if (m_spriteTextures.find(ecs::SpriteId::POWERUP) != m_spriteTextures.end()) {
        m_spriteTextures[spriteId] = m_spriteTextures[ecs::SpriteId::POWERUP];
        std::cout << "[PlayingState] ✓ Using powerup.png for ruban" << i << " (fallback after error)" << '\n';
      }
    }
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
  constexpr int EXPECTED_TEXTURE_COUNT = 39;
  std::cout << "[PlayingState] Successfully loaded " << m_spriteTextures.size() << " / " << EXPECTED_TEXTURE_COUNT
            << " sprite textures" << '\n';
  if (m_spriteTextures.size() < EXPECTED_TEXTURE_COUNT) {
    std::cerr << "[PlayingState] Missing textures will use fallback colored rectangles" << '\n';
  }

  // CHARGED_PROJECTILE = 6
  try {
    void *charged_projectile_tex = renderer->loadTexture("client/assets/sprites/chargedShot.png");
    if (charged_projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::CHARGED_PROJECTILE] = charged_projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded charged_projectile.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load charged_projectile.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load charged_projectile.png: " << e.what() << '\n';
  }

  // LOAD_CHARGED_SHOT = 8
  try {
    void *charged_projectile_tex = renderer->loadTexture("client/assets/sprites/loadChargedShot.png");
    if (charged_projectile_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::LOADING_SHOT] = charged_projectile_tex;
      std::cout << "[PlayingState] ✓ Loaded loadChargedShot.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load loadChargedShot.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load loadChargedShot.png: " << e.what() << '\n';
  }

  // DEATH_ANIM = 65 (spritesheet: 586x94, 6 frames)
  try {
    void *death_anim_tex = renderer->loadTexture("client/assets/sprites/death_anim.png");
    if (death_anim_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::DEATH_ANIM] = death_anim_tex;
      std::cout << "[PlayingState] ✓ Loaded death_anim.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load death_anim.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load death_anim.png: " << e.what() << '\n';
  }
}

void PlayingState::freeSpriteTextures()
{
  if (renderer == nullptr || m_spriteTextures.empty()) {
    return;
  }

  std::unordered_set<void *> destroyedTextures;

  for (auto &[spriteId, texture] : m_spriteTextures) {
    if (texture != nullptr && destroyedTextures.find(texture) == destroyedTextures.end()) {
      renderer->freeTexture(texture);
      destroyedTextures.insert(texture);
    }
  }

  m_spriteTextures.clear();
}
