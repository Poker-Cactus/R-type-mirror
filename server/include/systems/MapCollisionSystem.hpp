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
// MapCollision system removed: stub kept for compatibility

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
public:
  void update(ecs::World & /*world*/, float /*deltaTime*/) override
  {
    // Map collision behavior removed; system intentionally does nothing
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    // No signature — system does not process entities
    return ecs::ComponentSignature();
  }
};

} // namespace server

#endif // SERVER_MAP_COLLISION_SYSTEM_HPP_
