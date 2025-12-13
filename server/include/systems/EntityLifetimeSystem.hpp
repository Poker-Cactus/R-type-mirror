/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EntityLifetimeSystem.hpp - Destroys entities whose Lifetime expired
*/

#ifndef SERVER_ENTITY_LIFETIME_SYSTEM_HPP_
#define SERVER_ENTITY_LIFETIME_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Lifetime.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

class EntityLifetimeSystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    std::vector<ecs::Entity> toDestroy;
    for (auto entity : entities) {
      auto &life = world.getComponent<ecs::Lifetime>(entity);
      life.remaining -= deltaTime;
      if (life.remaining <= 0.0F) {
        toDestroy.push_back(entity);
      }
    }

    for (auto e : toDestroy) {
      world.destroyEntity(e);
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Lifetime>());
    return sig;
  }
};

} // namespace server

#endif // SERVER_ENTITY_LIFETIME_SYSTEM_HPP_
