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
#include "../../../engineCore/include/ecs/components/Powerup.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <iostream>
#include <vector>

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
  static constexpr float BUBBLE_OFFSET_X = 50.0f; // Behind the player
  static constexpr float BUBBLE_OFFSET_Y = 20.0f; // Slightly above
  static constexpr float BUBBLE_SMOOTHING = 10.0f;
  static constexpr unsigned int BUBBLE_SPRITE_WIDTH = 289; // bubble.png width (actual)
  static constexpr unsigned int BUBBLE_SPRITE_HEIGHT = 24; // bubble.png height
  static constexpr unsigned int BUBBLE_FRAME_COUNT = 12; // 12 frames of 24px each
  static constexpr int BUBBLE_FRAME_WIDTH = 24; // Frame width in pixels
  static constexpr float BUBBLE_SCALE = 2.5f;

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

    if (world.hasComponent<ecs::Powerup>(entityA) && world.hasComponent<ecs::Input>(entityB)) {
      powerupEntity = entityA;
      playerEntity = entityB;
    } else if (world.hasComponent<ecs::Powerup>(entityB) && world.hasComponent<ecs::Input>(entityA)) {
      powerupEntity = entityB;
      playerEntity = entityA;
    } else {
      return; // Not a player-powerup collision
    }

    const auto &powerup = world.getComponent<ecs::Powerup>(powerupEntity);

    std::cout << "[PowerupSystem] Player " << playerEntity << " collected powerup type "
              << static_cast<int>(powerup.type) << '\n';

    // Get powerup position before destroying it (for BUBBLE spawn location)
    float powerupY = 0.0f;
    if (world.hasComponent<ecs::Transform>(powerupEntity)) {
      powerupY = world.getComponent<ecs::Transform>(powerupEntity).y;
    }

    // Apply power-up effect based on type
    switch (powerup.type) {
    case ecs::PowerupType::DRONE:
      spawnDroneFollower(world, playerEntity);
      break;
    case ecs::PowerupType::BUBBLE:
      spawnBubbleFollower(world, playerEntity, powerupY);
      break;
    case ecs::PowerupType::BUBBLE_TRIPLE:
      // TODO: Implement shield
      break;
    case ecs::PowerupType::BUBBLE_RUBAN:
      // TODO: Implement weapon upgrade
      break;
    }

    // Destroy the power-up entity after collection
    world.destroyEntity(powerupEntity);
  }

  void spawnBubbleFollower(ecs::World &world, ecs::Entity player, float spawnY)
  {
    if (!world.hasComponent<ecs::Transform>(player)) {
      return;
    }

    const auto &playerTransform = world.getComponent<ecs::Transform>(player);

    ecs::Entity bubble = world.createEntity();

    // Transform - start at left side of screen at powerup Y position
    ecs::Transform transform;
    transform.x = 0.0f; // Spawn from left edge of screen
    transform.y = spawnY; // Use powerup's Y position
    transform.rotation = 0.0f;
    transform.scale = BUBBLE_SCALE;
    world.addComponent(bubble, transform);

    // Follower component - links to player, moves to ship's front tip
    ecs::Follower follower;
    follower.parent = player;
    follower.offsetX = 120.0f; // A bit more to the right
    follower.offsetY = 10.0f; // A bit lower
    world.addComponent(bubble, follower);

    // Sprite - use second line of spritesheet (row 1)
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::BUBBLE;
    sprite.width = BUBBLE_FRAME_WIDTH; // Width of one frame
    sprite.height = BUBBLE_SPRITE_HEIGHT; // Height of one row (not total spritesheet)
    sprite.animated = true;
    sprite.frameCount = BUBBLE_FRAME_COUNT;
    sprite.startFrame = 0;
    sprite.endFrame = BUBBLE_FRAME_COUNT - 1; // 0 to 11 (12 frames)
    sprite.currentFrame = 0;
    sprite.frameTime = 0.08f; // Slightly faster animation
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
};

} // namespace server

#endif // SERVER_POWERUP_SYSTEM_HPP_
