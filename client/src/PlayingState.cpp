/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.cpp
*/

#include "PlayingState.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
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
  if (!renderer) {
    std::cerr << "PlayingState: Renderer is null" << std::endl;
    return false;
  }

  // Initialiser le background parallaxe
  background = std::make_unique<ParallaxBackground>(renderer);
  if (!background->init()) {
    std::cerr << "PlayingState: Failed to initialize parallax background" << std::endl;
    return false;
  }

  // Load sprite textures
  loadSpriteTextures();

  std::cout << "PlayingState: Initialized successfully" << std::endl;

  return true;
}

void PlayingState::update(float dt)
{
  // Mettre à jour le background
  if (background) {
    background->update(dt);
  }

  // TODO: Mettre à jour les systèmes de jeu
  // - Mettre à jour les entités
  // - Vérifier les collisions
  // - Spawn des ennemis
  // - Mettre à jour l'IA
  // - Mettre à jour les projectiles
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
  
  if (!world || !renderer) {
    return;
  }

  // Render all entities that have Transform + Sprite
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Transform>());
  sig.set(ecs::getComponentId<ecs::Sprite>());

  std::vector<ecs::Entity> entities;
  world->getEntitiesWithSignature(sig, entities);

  for (auto entity : entities) {
    const auto &t = world->getComponent<ecs::Transform>(entity);
    const auto &sprite = world->getComponent<ecs::Sprite>(entity);

    // Try to use texture if available, otherwise fall back to colored rectangle
    auto textureIt = m_spriteTextures.find(sprite.spriteId);
    
    if (textureIt != m_spriteTextures.end() && textureIt->second != nullptr) {
      // Draw using actual texture
      if (sprite.spriteId == ecs::SpriteId::PLAYER_SHIP) {
        // Player ship is a spritesheet: 2450x150 with 7 frames
        // Each frame is 350x150 (2450/7 = 350)
        // Extract only the first frame (x=0, y=0, w=350, h=150)
        renderer->drawTextureRegion(textureIt->second,
                                   0, 0, 350, 150,  // Source: first frame
                                   static_cast<int>(t.x), 
                                   static_cast<int>(t.y), 
                                   static_cast<int>(sprite.width), 
                                   static_cast<int>(sprite.height)); // Destination
      } else if (sprite.spriteId == ecs::SpriteId::PROJECTILE) {
        // Projectile is a spritesheet: 422x92 with 2 frames
        // Each frame is 211x92 (422/2 = 211)
        // Extract only the first frame (x=0, y=0, w=211, h=92)
        renderer->drawTextureRegion(textureIt->second,
                                   0, 0, 211, 92,  // Source: first frame
                                   static_cast<int>(t.x), 
                                   static_cast<int>(t.y), 
                                   static_cast<int>(sprite.width), 
                                   static_cast<int>(sprite.height)); // Destination
      } else {
        // Other sprites: draw full texture
        renderer->drawTextureEx(textureIt->second, 
                               static_cast<int>(t.x), 
                               static_cast<int>(t.y), 
                               static_cast<int>(sprite.width), 
                               static_cast<int>(sprite.height),
                               0.0, false, false);
      }
    } else {
      // Fallback: colored rectangles for missing textures
      Color color{255, 255, 255, 255};
      
      switch (sprite.spriteId) {
      case ecs::SpriteId::PLAYER_SHIP:
        color = {100, 150, 255, 255}; // Blue
        break;
      case ecs::SpriteId::ENEMY_SHIP:
        color = {255, 100, 100, 255}; // Red
        break;
      case ecs::SpriteId::PROJECTILE:
        color = {255, 255, 100, 255}; // Yellow
        break;
      case ecs::SpriteId::POWERUP:
        color = {100, 255, 100, 255}; // Green
        break;
      case ecs::SpriteId::EXPLOSION:
        color = {255, 150, 50, 255}; // Orange
        break;
      default:
        color = {200, 200, 200, 255}; // Gray
        break;
      }

      renderer->drawRect(static_cast<int>(t.x), static_cast<int>(t.y), 
                        static_cast<int>(sprite.width), static_cast<int>(sprite.height), 
                        color);
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

  std::cout << "PlayingState: Cleaned up" << std::endl;
}

void PlayingState::loadSpriteTextures()
{
  if (!renderer) {
    return;
  }

  std::cout << "[PlayingState] Loading sprite textures..." << std::endl;
  
  // Load each texture individually with error handling
  // PLAYER_SHIP = 1 (spritesheet: 2450x150, 7 frames, using first frame only)
  try {
    void* playerTex = renderer->loadTexture("client/assets/sprites/player_ship.png");
    if (playerTex) {
      m_spriteTextures[ecs::SpriteId::PLAYER_SHIP] = playerTex;
      std::cout << "[PlayingState] ✓ Loaded player_ship.png" << std::endl;
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load player_ship.png (returned null)" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load player_ship.png: " << e.what() << std::endl;
  }
  
  // ENEMY_SHIP = 2
  try {
    void* enemyTex = renderer->loadTexture("client/assets/sprites/enemy_ship.png");
    if (enemyTex) {
      m_spriteTextures[ecs::SpriteId::ENEMY_SHIP] = enemyTex;
      std::cout << "[PlayingState] ✓ Loaded enemy_ship.png" << std::endl;
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.png (returned null)" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load enemy_ship.png: " << e.what() << std::endl;
  }
  
  // PROJECTILE = 3 (spritesheet: 422x92, 2 frames, using first frame only)
  try {
    void* projectileTex = renderer->loadTexture("client/assets/sprites/projectile.png");
    if (projectileTex) {
      m_spriteTextures[ecs::SpriteId::PROJECTILE] = projectileTex;
      std::cout << "[PlayingState] ✓ Loaded projectile.png" << std::endl;
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load projectile.png (returned null)" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load projectile.png: " << e.what() << std::endl;
  }
  
  // POWERUP = 5
  try {
    void* powerupTex = renderer->loadTexture("client/assets/sprites/powerup.png");
    if (powerupTex) {
      m_spriteTextures[ecs::SpriteId::POWERUP] = powerupTex;
      std::cout << "[PlayingState] ✓ Loaded powerup.png" << std::endl;
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load powerup.png (returned null)" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load powerup.png: " << e.what() << std::endl;
  }
  
  // EXPLOSION = 4
  try {
    void* explosionTex = renderer->loadTexture("client/assets/sprites/explosion.png");
    if (explosionTex) {
      m_spriteTextures[ecs::SpriteId::EXPLOSION] = explosionTex;
      std::cout << "[PlayingState] ✓ Loaded explosion.png" << std::endl;
    } else {
      std::cerr << "[PlayingState] ✗ Failed to load explosion.png (returned null)" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[PlayingState] ✗ Failed to load explosion.png: " << e.what() << std::endl;
  }
  
  std::cout << "[PlayingState] Successfully loaded " << m_spriteTextures.size() << " / 5 sprite textures" << std::endl;
  if (m_spriteTextures.size() < 5) {
    std::cerr << "[PlayingState] Missing textures will use fallback colored rectangles" << std::endl;
  }
}

void PlayingState::freeSpriteTextures()
{
  if (!renderer) {
    return;
  }

  for (auto &[spriteId, texture] : m_spriteTextures) {
    if (texture) {
      renderer->freeTexture(texture);
    }
  }
  
  m_spriteTextures.clear();
}
