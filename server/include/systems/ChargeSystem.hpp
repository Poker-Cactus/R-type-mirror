/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ChargeSystem.hpp - Updates loading shot position to follow owner
*/

#ifndef SERVER_CHARGE_SYSTEM_HPP_
#define SERVER_CHARGE_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Charging.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that makes loading shot animations follow their owner
 */
class ChargeSystem : public ecs::ISystem
{
public:
  ChargeSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    // Find all entities that are currently charging
    ecs::ComponentSignature chargingSig;
    chargingSig.set(ecs::getComponentId<ecs::Charging>());
    chargingSig.set(ecs::getComponentId<ecs::Transform>());

    std::vector<ecs::Entity> chargingEntities;
    world.getEntitiesWithSignature(chargingSig, chargingEntities);

    for (auto entity : chargingEntities) {
      const auto &charging = world.getComponent<ecs::Charging>(entity);

      // Skip if not charging or no loading shot entity
      if (!charging.isCharging || charging.loadingShotEntity == 0) {
        continue;
      }

      // Check if loading shot entity still exists
      if (!world.isAlive(charging.loadingShotEntity)) {
        continue;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    // System doesn't need a signature since it queries manually
    return {};
  }
};

} // namespace server

#endif // SERVER_CHARGE_SYSTEM_HPP_
