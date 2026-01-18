/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** MapCollisionSystem.hpp - System for handling map collisions
*/

#ifndef SERVER_MAP_COLLISION_SYSTEM_HPP_
#define SERVER_MAP_COLLISION_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/MapCollision.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include <iostream>

namespace server
{

/**
 * @brief System that handles collisions between entities and the map
 * 
 * The map is scaled to fill the game viewport height (1080px by default).
 * Collisions must be checked in the scaled coordinate space.
 */
class MapCollisionSystem : public ecs::ISystem
{
private:
  static constexpr float GAME_HEIGHT = 1080.0f; ///< Game viewport height

public:
  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    // Get the map collision entity (should be only one)
    ecs::Entity mapEntity = 0;
    const ecs::MapCollision *mapCollision = nullptr;

    // Find the map collision entity
    ecs::ComponentSignature mapSig;
    mapSig.set(ecs::getComponentId<ecs::MapCollision>());
    std::vector<ecs::Entity> mapEntities;
    world.getEntitiesWithSignature(mapSig, mapEntities);

    if (!mapEntities.empty()) {
      mapEntity = mapEntities[0];
      mapCollision = &world.getComponent<ecs::MapCollision>(mapEntity);
    }

    if (mapEntity == 0 || !mapCollision) {
      // No map collision data available
      return;
    }

    // Calculate scale factor: map is scaled to fill game height
    // Map pixel height = mapHeight * tileHeight
    const float mapPixelHeight = static_cast<float>(mapCollision->mapHeight * mapCollision->tileHeight);
    const float scale = GAME_HEIGHT / mapPixelHeight;

    // Check all entities with Transform, Collider, and Velocity
    ecs::ComponentSignature signature;
    signature.set(ecs::getComponentId<ecs::Transform>());
    signature.set(ecs::getComponentId<ecs::Collider>());
    signature.set(ecs::getComponentId<ecs::Velocity>());

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(signature, entities);

    for (const auto &entity : entities) {
      if (entity == mapEntity) {
        continue; // Skip the map entity itself
      }

      auto &transform = world.getComponent<ecs::Transform>(entity);
      auto &collider = world.getComponent<ecs::Collider>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);

      // Convert entity coordinates to tilemap space (unscale)
      const float tileX = transform.x / scale;
      const float tileY = transform.y / scale;
      const float tileNextX = (transform.x + velocity.dx * deltaTime) / scale;
      const float tileNextY = (transform.y + velocity.dy * deltaTime) / scale;
      const float tileWidth = collider.width / scale;
      const float tileHeight = collider.height / scale;

      // Check collision at projected position in tilemap space
      bool wouldCollide = mapCollision->checkRectCollision(tileNextX, tileNextY, tileWidth, tileHeight);

      if (wouldCollide) {
        // Try horizontal movement only (slide along Y wall)
        bool horizontalCollision = mapCollision->checkRectCollision(tileNextX, tileY, tileWidth, tileHeight);
        
        // Try vertical movement only (slide along X wall)
        bool verticalCollision = mapCollision->checkRectCollision(tileX, tileNextY, tileWidth, tileHeight);

        // Allow sliding movement - cancel only the axis that collides
        if (horizontalCollision) {
          velocity.dx = 0.0F; // Can't move horizontally
        }
        
        if (verticalCollision) {
          velocity.dy = 0.0F; // Can't move vertically
        }
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature signature;
    signature.set(ecs::getComponentId<ecs::MapCollision>());
    return signature;
  }
};

} // namespace server

#endif // SERVER_MAP_COLLISION_SYSTEM_HPP_
