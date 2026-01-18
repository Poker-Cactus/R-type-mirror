/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PowerupSystem.hpp - Handles power-up collection and effects
*/

#ifndef SERVER_POWERUP_SYSTEM_HPP_
#define SERVER_POWERUP_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <iostream>
#include <vector>

typedef struct BubbleTypeConfig {
  unsigned int spriteWidth;
  unsigned int spriteHeight;
  unsigned int frameCount;
  int frameWidth;
  float scale;
  std::uint32_t spriteId;
} BubbleTypeConfig;

namespace server
{

/**
 * @brief System that handles power-up collection via collision events
 *
 * When a player collides with a power-up, this system applies
 * the power-up effect (e.g., spawning a drone follower).
 */
class PowerupSystem : public ecs::ISystem
{
public:
  PowerupSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;
    (void)world;
    // This system works via event subscription, see initialize()
  }

  /**
   * @brief Subscribe to collision events to detect power-up collection
   */
  void initialize(ecs::World &world)
  {
    m_collisionHandle = world.subscribeEvent<ecs::CollisionEvent>(
      [this, &world](const ecs::CollisionEvent &event) { handleCollision(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

private:
  ecs::EventListenerHandle m_collisionHandle;

  // Drone follower configuration
  static constexpr float DRONE_OFFSET_X = -50.0f; // Behind the player
  static constexpr float DRONE_OFFSET_Y = -20.0f; // Slightly above
  static constexpr float DRONE_SMOOTHING = 10.0f;
  static constexpr unsigned int DRONE_SPRITE_WIDTH = 205;
  static constexpr unsigned int DRONE_SPRITE_HEIGHT = 18;
  static constexpr unsigned int DRONE_FRAME_COUNT = 12;
  static constexpr int DRONE_FRAME_WIDTH = DRONE_SPRITE_WIDTH / DRONE_FRAME_COUNT; // 17px (integer division)
  static constexpr float DRONE_SCALE = 2.5f;

  // BUBBLE follower configuration
  static constexpr unsigned int BUBBLE_SPRITE_WIDTH = 289; // bubble.png width (actual)
  static constexpr unsigned int BUBBLE_SPRITE_HEIGHT = 24; // bubble.png height
  static constexpr unsigned int BUBBLE_FRAME_COUNT = 12; // 12 frames of 24px each
  static constexpr int BUBBLE_FRAME_WIDTH = 24; // Frame width in pixels
  static constexpr float BUBBLE_SCALE = 2.5f;

  static constexpr unsigned int BUBBLE_TRIPLE_SPRITE_WIDTH = 181; // bubble.png width (actual)
  static constexpr unsigned int BUBBLE_TRIPLE_SPRITE_HEIGHT = 23; // bubble.png height
  static constexpr unsigned int BUBBLE_TRIPLE_FRAME_COUNT = 6;
  static constexpr int BUBBLE_TRIPLE_FRAME_WIDTH =
    BUBBLE_TRIPLE_SPRITE_WIDTH / BUBBLE_TRIPLE_FRAME_COUNT; // Frame width in pixels
  static constexpr float BUBBLE_TRIPLE_SCALE = 2.5f;

  // bubble_ruban_back: 32x32 - individual frames (4 separate files)
  static constexpr unsigned int BUBBLE_RUBAN1_SPRITE_WIDTH = 32;
  static constexpr unsigned int BUBBLE_RUBAN1_SPRITE_HEIGHT = 32;
  static constexpr unsigned int BUBBLE_RUBAN1_FRAME_COUNT = 4;
  static constexpr int BUBBLE_RUBAN1_FRAME_WIDTH = 32; // Each file is a single frame
  static constexpr float BUBBLE_RUBAN1_SCALE = 2.5f;

  // bubble_ruban_middle: 38x29 - individual frames (4 separate files)
  static constexpr unsigned int BUBBLE_RUBAN2_SPRITE_WIDTH = 38;
  static constexpr unsigned int BUBBLE_RUBAN2_SPRITE_HEIGHT = 29;
  static constexpr unsigned int BUBBLE_RUBAN2_FRAME_COUNT = 4;
  static constexpr int BUBBLE_RUBAN2_FRAME_WIDTH = 38; // Each file is a single frame
  static constexpr float BUBBLE_RUBAN2_SCALE = 2.5f;

  // bubble_ruban_front: 37x31 - individual frames (4 separate files)
  static constexpr unsigned int BUBBLE_RUBAN3_SPRITE_WIDTH = 37;
  static constexpr unsigned int BUBBLE_RUBAN3_SPRITE_HEIGHT = 31;
  static constexpr unsigned int BUBBLE_RUBAN3_FRAME_COUNT = 4;
  static constexpr int BUBBLE_RUBAN3_FRAME_WIDTH = 37; // Each file is a single frame
  static constexpr float BUBBLE_RUBAN3_SCALE = 2.5f;

  void handleCollision(ecs::World &world, const ecs::CollisionEvent &event)
  {
    ecs::Entity entityA = event.entityA;
    ecs::Entity entityB = event.entityB;

    if (!world.isAlive(entityA) || !world.isAlive(entityB)) {
      return;
    }

    // Determine which entity is the power-up and which is the player
    ecs::Entity powerupEntity = 0;
    ecs::Entity playerEntity = 0;

    // Helper to check if sprite is a collectible bubble/powerup
    auto isCollectibleSprite = [](std::uint32_t spriteId) {
      return spriteId == ecs::SpriteId::POWERUP || spriteId == ecs::SpriteId::BUBBLE ||
        spriteId == ecs::SpriteId::BUBBLE_TRIPLE || spriteId == ecs::SpriteId::DRONE ||
        (spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 && spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
        (spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 && spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4);
    };

    // Check if one entity is a collectible sprite (without Follower = detached) and the other is a player
    if (world.hasComponent<ecs::Sprite>(entityA) && world.hasComponent<ecs::Input>(entityB)) {
      const auto &sprite = world.getComponent<ecs::Sprite>(entityA);
      // Collectible if it's a POWERUP, or a bubble without Follower (detached)
      bool isCollectible = (sprite.spriteId == ecs::SpriteId::POWERUP) ||
        (isCollectibleSprite(sprite.spriteId) && !world.hasComponent<ecs::Follower>(entityA));
      if (isCollectible) {
        powerupEntity = entityA;
        playerEntity = entityB;
      }
    } else if (world.hasComponent<ecs::Sprite>(entityB) && world.hasComponent<ecs::Input>(entityA)) {
      const auto &sprite = world.getComponent<ecs::Sprite>(entityB);
      // Collectible if it's a POWERUP, or a bubble without Follower (detached)
      bool isCollectible = (sprite.spriteId == ecs::SpriteId::POWERUP) ||
        (isCollectibleSprite(sprite.spriteId) && !world.hasComponent<ecs::Follower>(entityB));
      if (isCollectible) {
        powerupEntity = entityB;
        playerEntity = entityA;
      }
    }

    if (powerupEntity == 0) {
      return; // Not a player-powerup collision
    }

    // Determine powerup type from sprite
    const auto &powerupSprite = world.getComponent<ecs::Sprite>(powerupEntity);
    std::cout << "[PowerupSystem] Player " << playerEntity << " collected powerup sprite " << powerupSprite.spriteId
              << '\n';

    // Get powerup position before destroying it (for BUBBLE spawn location)
    float powerupX = 0.0f;
    float powerupY = 0.0f;
    if (world.hasComponent<ecs::Transform>(powerupEntity)) {
      const auto &powerupTransform = world.getComponent<ecs::Transform>(powerupEntity);
      powerupX = powerupTransform.x;
      powerupY = powerupTransform.y;
    }
    BubbleTypeConfig config;

    // Determine bubble type - either from POWERUP frame or directly from sprite ID
    int bubbleType = -1;
    if (powerupSprite.spriteId == ecs::SpriteId::POWERUP) {
      // Standard powerup pickup - use frame to determine type
      bubbleType = static_cast<int>(powerupSprite.currentFrame);
    } else if (powerupSprite.spriteId == ecs::SpriteId::BUBBLE) {
      bubbleType = 0;
    } else if (powerupSprite.spriteId == ecs::SpriteId::BUBBLE_TRIPLE) {
      bubbleType = 1;
    } else if (powerupSprite.spriteId == ecs::SpriteId::DRONE) {
      bubbleType = 3;
    } else if ((powerupSprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 &&
                powerupSprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
               (powerupSprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 &&
                powerupSprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4)) {
      bubbleType = 2;
    }

    // Apply power-up effect based on type
    switch (bubbleType) {
    case 3: // DRONE
      spawnDroneFollower(world, playerEntity);
      break;
    case 0: // BUBBLE
      config.spriteWidth = BUBBLE_SPRITE_WIDTH;
      config.spriteHeight = BUBBLE_SPRITE_HEIGHT;
      config.frameCount = BUBBLE_FRAME_COUNT;
      config.frameWidth = BUBBLE_FRAME_WIDTH;
      config.scale = BUBBLE_SCALE;
      config.spriteId = ecs::SpriteId::BUBBLE;
      spawnBubbleFollower(world, playerEntity, powerupX, powerupY, config);
      break;
    case 1: // BUBBLE_TRIPLE
      config.spriteWidth = BUBBLE_TRIPLE_SPRITE_WIDTH;
      config.spriteHeight = BUBBLE_TRIPLE_SPRITE_HEIGHT;
      config.frameCount = BUBBLE_TRIPLE_FRAME_COUNT;
      config.frameWidth = BUBBLE_TRIPLE_FRAME_WIDTH;
      config.scale = BUBBLE_TRIPLE_SCALE;
      config.spriteId = ecs::SpriteId::BUBBLE_TRIPLE;
      spawnBubbleFollower(world, playerEntity, powerupX, powerupY, config);
      break;
    case 2: // BUBBLE_RUBAN - starts with first middle frame
      config.spriteWidth = BUBBLE_RUBAN2_SPRITE_WIDTH;
      config.spriteHeight = BUBBLE_RUBAN2_SPRITE_HEIGHT;
      config.frameCount = BUBBLE_RUBAN2_FRAME_COUNT;
      config.frameWidth = BUBBLE_RUBAN2_FRAME_WIDTH;
      config.scale = BUBBLE_RUBAN2_SCALE;
      config.spriteId = ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1; // Start with first middle frame
      spawnBubbleFollower(world, playerEntity, powerupX, powerupY, config);
      break;
    default:
      break;
    }

    // Destroy the power-up entity after collection
    world.destroyEntity(powerupEntity);
  }

  void spawnBubbleFollower(ecs::World &world, ecs::Entity player, float spawnX, float spawnY,
                           BubbleTypeConfig bubbleConfig)
  {
    if (!world.hasComponent<ecs::Transform>(player)) {
      return;
    }

    // First, destroy any existing bubble follower for this player
    destroyExistingBubbleFollower(world, player);

    ecs::Entity bubble = world.createEntity();

    // Transform - spawn at powerup position
    ecs::Transform transform;
    transform.x = spawnX; // Spawn where powerup was collected
    transform.y = spawnY; // Use powerup's Y position
    transform.rotation = 0.0f;
    transform.scale = bubbleConfig.scale;
    world.addComponent(bubble, transform);

    // Follower component - links to player, moves to ship's front tip
    ecs::Follower follower;
    follower.parent = player;
    follower.offsetX = 120.0f; // A bit to the right
    follower.offsetY = 10.0f; // A bit lower
    follower.type = bubbleConfig.spriteId;
    world.addComponent(bubble, follower);

    // Sprite
    ecs::Sprite sprite;
    sprite.spriteId = bubbleConfig.spriteId;
    sprite.width = bubbleConfig.frameWidth; // Width of one frame
    sprite.height = bubbleConfig.spriteHeight;
    sprite.animated = true;
    sprite.frameCount = bubbleConfig.frameCount;
    sprite.startFrame = 0;
    sprite.endFrame = bubbleConfig.frameCount - 1; // 0 to 11 (12 frames)
    sprite.currentFrame = 0;
    sprite.frameTime = 0.1f; // Moderate animation speed
    sprite.reverseAnimation = false; // Play forward to avoid loop cut
    sprite.loop = true;
    sprite.row = 0; // bubble.png has only one row
    sprite.offsetX = 0; // No horizontal offset needed
    world.addComponent(bubble, sprite);

    // Networked so clients can see the bubble
    ecs::Networked net;
    net.networkId = bubble;
    world.addComponent(bubble, net);

    std::cout << "[PowerupSystem] Spawned bubble follower " << bubble << " for player " << player << " at y=" << spawnY
              << " (moving to ship tip)\n";
  }

  void spawnDroneFollower(ecs::World &world, ecs::Entity player)
  {
    if (!world.hasComponent<ecs::Transform>(player)) {
      return;
    }

    const auto &playerTransform = world.getComponent<ecs::Transform>(player);

    // Count existing drones for this player to offset new ones
    int followerCount = countPlayerDrones(world, player);
    float yOffset = DRONE_OFFSET_Y + (followerCount * -25.0f); // Stack drones vertically

    ecs::Entity drone = world.createEntity();

    // Transform - start at player position
    ecs::Transform transform;
    transform.x = playerTransform.x + DRONE_OFFSET_X;
    transform.y = playerTransform.y + yOffset;
    transform.rotation = 0.0f;
    transform.scale = DRONE_SCALE;
    world.addComponent(drone, transform);

    // Follower component - links to player
    ecs::Follower follower;
    follower.parent = player;
    follower.offsetX = DRONE_OFFSET_X;
    follower.offsetY = yOffset;
    follower.smoothing = DRONE_SMOOTHING;
    follower.type = ecs::SpriteId::DRONE; // Drone type
    world.addComponent(drone, follower);

    // No collider for drones - they should not interact with physics

    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::DRONE;
    sprite.width = DRONE_FRAME_WIDTH;
    sprite.height = DRONE_SPRITE_HEIGHT;
    sprite.animated = true;
    sprite.frameCount = DRONE_FRAME_COUNT;
    sprite.startFrame = 11;
    sprite.endFrame = 0;
    sprite.currentFrame = 11;
    sprite.frameTime = 0.1f;
    sprite.reverseAnimation = true;
    sprite.loop = true;
    world.addComponent(drone, sprite);

    // Networked so clients can see the drone
    ecs::Networked net;
    net.networkId = drone;
    world.addComponent(drone, net);

    std::cout << "[PowerupSystem] Spawned drone follower " << drone << " for player " << player << " (drone #"
              << (followerCount + 1) << ")\n";
  }

  static int countPlayerDrones(ecs::World &world, ecs::Entity player)
  {
    int count = 0;
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Follower>());

    std::vector<ecs::Entity> followers;
    world.getEntitiesWithSignature(sig, followers);

    for (const auto &entity : followers) {
      if (world.isAlive(entity)) {
        const auto &follower = world.getComponent<ecs::Follower>(entity);
        if (follower.parent == player) {
          count++;
        }
      }
    }
    return count;
  }

  /**
   * @brief Destroy existing bubble follower for a player
   * Checks if the follower has a bubble sprite (BUBBLE, BUBBLE_TRIPLE, or BUBBLE_RUBAN*)
   */
  void destroyExistingBubbleFollower(ecs::World &world, ecs::Entity player)
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Follower>());
    sig.set(ecs::getComponentId<ecs::Sprite>());

    std::vector<ecs::Entity> followers;
    world.getEntitiesWithSignature(sig, followers);

    for (const auto &entity : followers) {
      if (world.isAlive(entity)) {
        const auto &follower = world.getComponent<ecs::Follower>(entity);
        if (follower.parent == player) {
          const auto &sprite = world.getComponent<ecs::Sprite>(entity);
          // Check for all bubble sprite IDs including all ruban animation frames
          bool isRubanSprite =
            (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 && sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
            (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 &&
             sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4);
          if (sprite.spriteId == ecs::SpriteId::BUBBLE || sprite.spriteId == ecs::SpriteId::BUBBLE_TRIPLE ||
              isRubanSprite) {
            std::cout << "[PowerupSystem] Destroying existing bubble follower " << entity
                      << " (spriteId: " << sprite.spriteId << ")\n";
            world.destroyEntity(entity);
            return; // Only destroy one bubble per player
          }
        }
      }
    }
  }
};

} // namespace server

#endif // SERVER_POWERUP_SYSTEM_HPP_
