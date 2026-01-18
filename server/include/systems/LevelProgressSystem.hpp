/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LevelProgressSystem.hpp - Tracks player progression through levels
*/

#ifndef SERVER_LEVEL_PROGRESS_SYSTEM_HPP_
#define SERVER_LEVEL_PROGRESS_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/LevelProgress.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include <iostream>
#include <vector>

namespace server
{

/**
 * @brief System that tracks player progression through a level
 * 
 * This system accumulates the distance traveled based on automatic scrolling,
 * similar to the parallax background. The world scrolls at a constant speed,
 * independent of player movement.
 */
class LevelProgressSystem : public ecs::ISystem
{
public:
  // Scroll speed in pixels per second (matching the parallax fast layer)
  static constexpr float SCROLL_SPEED = 100.0f;
  
  LevelProgressSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    // Find all players with LevelProgress component
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    playerSig.set(ecs::getComponentId<ecs::LevelProgress>());
    
    std::vector<ecs::Entity> players;
    world.getEntitiesWithSignature(playerSig, players);
    
    if (players.empty()) {
      return;
    }
    
    // Calculate distance traveled this frame based on automatic scrolling
    float distanceThisFrame = SCROLL_SPEED * deltaTime;
    
    // Update all players' progress (they all progress at the same rate)
    for (auto player : players) {
      auto &progress = world.getComponent<ecs::LevelProgress>(player);
      progress.distanceTraveled += distanceThisFrame;
      
      // Log every 500 units for visibility
      static float lastLoggedDistance = 0;
      if (progress.distanceTraveled - lastLoggedDistance >= 500) {
        std::cout << "[LevelProgress] Distance traveled: " << progress.distanceTraveled 
                  << " px (scroll speed: " << SCROLL_SPEED << " px/s)" << std::endl;
        lastLoggedDistance = progress.distanceTraveled;
      }
    }
  }

  ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature signature;
    signature.set(ecs::getComponentId<ecs::PlayerId>());
    signature.set(ecs::getComponentId<ecs::LevelProgress>());
    return signature;
  }
};

} // namespace server

#endif // SERVER_LEVEL_PROGRESS_SYSTEM_HPP_
