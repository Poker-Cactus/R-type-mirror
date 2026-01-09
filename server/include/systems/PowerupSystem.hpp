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
  static constexpr float BUBBLE_OFFSET_X = -50.0f; // Behind the player
  static constexpr float BUBBLE_OFFSET_Y = -20.0f; // Slightly above
  static constexpr float BUBBLE_SMOOTHING = 10.0f;
  static constexpr unsigned int BUBBLE_SPRITE_WIDTH = 600;
  static constexpr unsigned int BUBBLE_SPRITE_HEIGHT = 866;
  static constexpr unsigned int BUBBLE_FRAME_COUNT = 12;
  static constexpr int BUBBLE_FRAME_WIDTH = BUBBLE_SPRITE_WIDTH / BUBBLE_FRAME_COUNT; // 50px (integer division)
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

    // Apply power-up effect based on type
    switch (powerup.type) {
    case ecs::PowerupType::DRONE:
      spawnDroneFollower(world, playerEntity);
      break;
    case ecs::PowerupType::BUBBLE:
      spawnBubbleFollower(world, playerEntity);
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
  void spawnBubbleFollower(ecs::World &world, ecs::Entity player)
  {
    if (!world.hasComponent<ecs::Transform>(player)) {
      return;
    }

    const auto &playerTransform = world.getComponent<ecs::Transform>(player);

    // Bubble follower appears to the left of the player's ship
    float bubbleOffsetX = -60.0f; // Adjust as needed for your sprite size
    float bubbleOffsetY = 0.0f; // Same vertical position as player

    ecs::Entity bubble = world.createEntity();

    // Transform - start at player position + offset
    ecs::Transform transform;
    transform.x = playerTransform.x + bubbleOffsetX;
    transform.y = playerTransform.y + bubbleOffsetY;
    transform.rotation = 0.0f;
    transform.scale = BUBBLE_SCALE;
    world.addComponent(bubble, transform);

    // Follower component - links to player, stays at left
    ecs::Follower follower;
    follower.parent = player;
    follower.offsetX = bubbleOffsetX;
    follower.offsetY = bubbleOffsetY;
    follower.smoothing = BUBBLE_SMOOTHING;
    world.addComponent(bubble, follower);

    // Sprite - use second line of spritesheet
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::BUBBLE;
    sprite.width = BUBBLE_FRAME_WIDTH;
    sprite.height = BUBBLE_SPRITE_HEIGHT / 2; // Use half height for one row
    sprite.animated = true;
    sprite.frameCount = BUBBLE_FRAME_COUNT;
    sprite.startFrame = 11;
    sprite.endFrame = 0;
    sprite.currentFrame = 11;
    sprite.frameTime = 0.1f;
    sprite.reverseAnimation = true;
    sprite.loop = true;
    // sprite.row = 1; // <-- Utilise la deuxième ligne (row index 1)
    world.addComponent(bubble, sprite);

    // Networked so clients can see the bubble
    ecs::Networked net;
    net.networkId = bubble;
    world.addComponent(bubble, net);

    std::cout << "[PowerupSystem] Spawned bubble follower " << bubble << " for player " << player << " (spritesheet row 2)\n";
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
