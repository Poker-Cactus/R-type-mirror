/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.cpp
*/

#include "PlayingState.hpp"
#include "../include/AssetPath.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/Geometry.hpp"
#include <iostream>

PlayingState::PlayingState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world)
    : renderer(renderer), world(world), background(nullptr)
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

  // Initialiser le background parallaxe
  background = std::make_unique<ParallaxBackground>(renderer);
  if (!background->init()) {
    std::cerr << "PlayingState: Failed to initialize parallax background" << '\n';
    return false;
  }

  // Load sprite textures
  loadSpriteTextures();

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

  std::cout << "PlayingState: Initialized successfully" << '\n';

  return true;
}

void PlayingState::update(float delta_time)
{
  // Mettre à jour le background
  if (background) {
    background->update(delta_time);
  }

  // Update HUD data from world state
  updateHUDFromWorld();
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
      constexpr int PLAYER_FRAME_WIDTH = 350;
      constexpr int PLAYER_FRAME_HEIGHT = 150;
      // Draw using actual texture
      if (sprite.spriteId == ecs::SpriteId::PLAYER_SHIP) {
        // Player ship is a spritesheet: 2450x150 with 7 frames
        // Each frame is 350x150 (2450/7 = 350)
        // Extract only the first frame (x=0, y=0, w=350, h=150)
        renderer->drawTextureRegion(
          textureIt->second,
          {.x = 0, .y = 0, .width = PLAYER_FRAME_WIDTH, .height = PLAYER_FRAME_HEIGHT}, // Source: first frame
          {.x = static_cast<int>(transformComponent.x),
           .y = static_cast<int>(transformComponent.y),
           .width = static_cast<int>(sprite.width),
           .height = static_cast<int>(sprite.height)}); // Destination
      } else if (sprite.spriteId == ecs::SpriteId::PROJECTILE) {
        // Projectile is a spritesheet: 422x92 with 2 frames
        // Each frame is 211x92 (422/2 = 211)
        // Extract only the first frame (x=0, y=0, w=211, h=92)
        constexpr int PROJECTILE_FRAME_WIDTH = 211;
        constexpr int PROJECTILE_FRAME_HEIGHT = 92;
        renderer->drawTextureRegion(
          textureIt->second,
          {.x = 0, .y = 0, .width = PROJECTILE_FRAME_WIDTH, .height = PROJECTILE_FRAME_HEIGHT}, // Source: first frame
          {.x = static_cast<int>(transformComponent.x),
           .y = static_cast<int>(transformComponent.y),
           .width = static_cast<int>(sprite.width),
           .height = static_cast<int>(sprite.height)}); // Destination
      } else {
        // Other sprites: draw full texture
        renderer->drawTextureEx(textureIt->second,
                                {.x = static_cast<int>(transformComponent.x),
                                 .y = static_cast<int>(transformComponent.y),
                                 .width = static_cast<int>(sprite.width),
                                 .height = static_cast<int>(sprite.height)},
                                0.0, false, false);
      }
    } else {
      // Fallback: colored rectangles for missing textures
      constexpr Color COLOR_WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
      constexpr Color COLOR_PLAYER_BLUE = {.r = 100, .g = 150, .b = 255, .a = 255};
      constexpr Color COLOR_ENEMY_RED = {.r = 255, .g = 100, .b = 100, .a = 255};
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

      renderer->drawRect(static_cast<int>(transformComponent.x), static_cast<int>(transformComponent.y),
                         static_cast<int>(sprite.width), static_cast<int>(sprite.height), color);
    }
  }

  // Draw HUD on top of everything
  renderHUD();
}

void PlayingState::renderHUD()
{
  if (renderer == nullptr || m_hudFont == nullptr) {
    return;
  }

  // Draw health bar
  constexpr int HEALTH_BAR_X = 20;
  constexpr int HEALTH_BAR_Y = 20;
  constexpr int HEALTH_BAR_WIDTH = 200;
  constexpr int HEALTH_BAR_HEIGHT = 20;
  constexpr int HUD_BACKGROUND_COLOR_R = 50;
  constexpr int HUD_BACKGROUND_COLOR_G = 50;
  constexpr int HUD_BACKGROUND_COLOR_B = 50;
  constexpr int HUD_BACKGROUND_ALPHA = 200;
  constexpr Color HUD_BACKGROUND_COLOR = {
    .r = HUD_BACKGROUND_COLOR_R, .g = HUD_BACKGROUND_COLOR_G, .b = HUD_BACKGROUND_COLOR_B, .a = HUD_BACKGROUND_ALPHA};
  constexpr Color HUD_HEALTH_GREEN = {.r = 100, .g = 255, .b = 100, .a = 255};
  constexpr Color HUD_HEALTH_YELLOW = {.r = 255, .g = 255, .b = 100, .a = 255};
  constexpr Color HUD_HEALTH_RED = {.r = 255, .g = 100, .b = 100, .a = 255};
  constexpr Color HUD_TEXT_WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
  constexpr int HEALTH_THRESHOLD_HIGH = 60;
  constexpr int HEALTH_THRESHOLD_MID = 30;
  constexpr int MAX_HEALTH = 100;
  constexpr int HUD_TEXT_OFFSET_X = 5;
  constexpr int HUD_TEXT_OFFSET_Y = 2;
  constexpr int HUD_SCORE_OFFSET_Y = 30;

  // Background (dark gray)
  renderer->drawRect(HEALTH_BAR_X, HEALTH_BAR_Y, HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT, HUD_BACKGROUND_COLOR);

  // Health fill (green to red based on health)
  int healthWidth = (m_playerHealth * HEALTH_BAR_WIDTH) / MAX_HEALTH;
  Color healthColor;
  if (m_playerHealth > HEALTH_THRESHOLD_HIGH) {
    healthColor = HUD_HEALTH_GREEN;
  } else if (m_playerHealth > HEALTH_THRESHOLD_MID) {
    healthColor = HUD_HEALTH_YELLOW;
  } else {
    healthColor = HUD_HEALTH_RED;
  }
  renderer->drawRect(HEALTH_BAR_X, HEALTH_BAR_Y, healthWidth, HEALTH_BAR_HEIGHT, healthColor);

  // Health text
  std::string healthText = "HP: " + std::to_string(m_playerHealth) + "/" + std::to_string(MAX_HEALTH);
  renderer->drawText(m_hudFont, healthText, HEALTH_BAR_X + HUD_TEXT_OFFSET_X, HEALTH_BAR_Y + HUD_TEXT_OFFSET_Y,
                     HUD_TEXT_WHITE);

  // Score text
  std::string scoreText = "Score: " + std::to_string(m_playerScore);
  renderer->drawText(m_hudFont, scoreText, HEALTH_BAR_X, HEALTH_BAR_Y + HUD_SCORE_OFFSET_Y, HUD_TEXT_WHITE);
}

void PlayingState::updateHUDFromWorld()
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
}

void PlayingState::processInput()
{
  // TODO: Gérer les entrées du joueur
  // - Mouvement du vaisseau
  // - Tir
  // - Pause (Échap)
}

void PlayingState::cleanup()
{
  if (background) {
    background->cleanup();
    background.reset();
  }

  // Free all loaded sprite textures
  freeSpriteTextures();

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
    void *player_tex = renderer->loadTexture("client/assets/sprites/player_ship.png");
    if (player_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::PLAYER_SHIP] = player_tex;
      std::cout << "[PlayingState] ✓ Loaded player_ship.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load player_ship.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load player_ship.png: " << e.what() << '\n';
  }

  // ENEMY_SHIP = 2
  try {
    void *enemy_tex = renderer->loadTexture("client/assets/sprites/enemy_ship.png");
    if (enemy_tex != nullptr) {
      m_spriteTextures[ecs::SpriteId::ENEMY_SHIP] = enemy_tex;
      std::cout << "[PlayingState] ✓ Loaded enemy_ship.png" << '\n';
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.png (returned null)" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.png: " << e.what() << '\n';
  }

  // PROJECTILE = 3 (spritesheet: 422x92, 2 frames, using first frame only)
  try {
    void *projectile_tex = renderer->loadTexture("client/assets/sprites/projectile.png");
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

  constexpr int EXPECTED_TEXTURE_COUNT = 5;
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
