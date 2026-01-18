/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** CollisionSystem.hpp - Handles collision detection and response
*/

#ifndef SERVER_COLLISION_SYSTEM_HPP_
#define SERVER_COLLISION_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace server
{

/**
 * @brief System that detects collisions and emits collision events
 */
class CollisionSystem : public ecs::ISystem
{
public:
  CollisionSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    // Check all pairs of entities
    for (size_t i = 0; i < entities.size(); ++i) {
      for (size_t j = i + 1; j < entities.size(); ++j) {
        ecs::Entity entityA = entities[i];
        ecs::Entity entityB = entities[j];

        if (!world.isAlive(entityA) || !world.isAlive(entityB)) {
          continue;
        }

        if (!world.hasComponent<ecs::Transform>(entityA) || !world.hasComponent<ecs::Transform>(entityB) ||
            !world.hasComponent<ecs::Collider>(entityA) || !world.hasComponent<ecs::Collider>(entityB)) {
          continue;
        }

        const auto &transformA = world.getComponent<ecs::Transform>(entityA);
        const auto &transformB = world.getComponent<ecs::Transform>(entityB);
        const auto &colliderA = world.getComponent<ecs::Collider>(entityA);
        const auto &colliderB = world.getComponent<ecs::Collider>(entityB);

        // Skip collision if both entities are enemies
        bool isEnemyA = isEnemy(world, entityA);
        bool isEnemyB = isEnemy(world, entityB);
        if (isEnemyA && isEnemyB) {
          continue; // Enemies don't collide with each other
        }

        if (checkCollision(transformA, colliderA, transformB, colliderB)) {
          // Emit collision event
          ecs::CollisionEvent event(entityA, entityB);
          world.emitEvent(event);
        }
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Transform>());
    sig.set(ecs::getComponentId<ecs::Collider>());
    return sig;
  }

private:
  /**
   * @brief Check if an entity is an enemy based on its sprite ID
   */
  static bool isEnemy(ecs::World &world, ecs::Entity entity)
  {
    if (!world.hasComponent<ecs::Sprite>(entity)) {
      return false;
    }
    const auto &sprite = world.getComponent<ecs::Sprite>(entity);
    return sprite.spriteId == ecs::SpriteId::ENEMY_SHIP || sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW ||
      sprite.spriteId == ecs::SpriteId::ENEMY_WALKER || sprite.spriteId == ecs::SpriteId::ENEMY_ROBOT ||
      sprite.spriteId == ecs::SpriteId::ELITE_ENEMY || sprite.spriteId == ecs::SpriteId::SHIELD_BUBBLE;
  }

  static bool checkCollision(const ecs::Transform &transA, const ecs::Collider &colA, const ecs::Transform &transB,
                             const ecs::Collider &colB)
  {
    if (colA.shape == ecs::Collider::Shape::BOX && colB.shape == ecs::Collider::Shape::BOX) {
      return checkBoxBox(transA, colA, transB, colB);
    }
    if (colA.shape == ecs::Collider::Shape::CIRCLE && colB.shape == ecs::Collider::Shape::CIRCLE) {
      return checkCircleCircle(transA, colA, transB, colB);
    }
    return checkBoxCircle(transA, colA, transB, colB);
  }

  static bool checkBoxBox(const ecs::Transform &transA, const ecs::Collider &colA, const ecs::Transform &transB,
                          const ecs::Collider &colB)
  {
    return transA.x < transB.x + colB.width && transA.x + colA.width > transB.x && transA.y < transB.y + colB.height &&
      transA.y + colA.height > transB.y;
  }

  static bool checkCircleCircle(const ecs::Transform &transA, const ecs::Collider &colA, const ecs::Transform &transB,
                                const ecs::Collider &colB)
  {
    float dx = transB.x - transA.x;
    float dy = transB.y - transA.y;
    float distance = std::sqrt((dx * dx) + (dy * dy));
    return distance < (colA.radius + colB.radius);
  }

  static bool checkBoxCircle(const ecs::Transform &transA, const ecs::Collider &colA, const ecs::Transform &transB,
                             const ecs::Collider &colB)
  {
    const ecs::Transform *box = &transA;
    const ecs::Collider *boxCol = &colA;
    const ecs::Transform *circle = &transB;
    const ecs::Collider *circleCol = &colB;

    if (colA.shape == ecs::Collider::Shape::CIRCLE) {
      box = &transB;
      boxCol = &colB;
      circle = &transA;
      circleCol = &colA;
    }

    float closestX = std::max(box->x, std::min(circle->x, box->x + boxCol->width));
    float closestY = std::max(box->y, std::min(circle->y, box->y + boxCol->height));

    float dx = circle->x - closestX;
    float dy = circle->y - closestY;
    float distance = std::sqrt((dx * dx) + (dy * dy));

    return distance < circleCol->radius;
  }
};

} // namespace server

#endif // SERVER_COLLISION_SYSTEM_HPP_
