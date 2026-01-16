/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAIUtility.cpp - Implementation of utility functions for ally AI
*/

#include "../../include/ai/AllyAIUtility.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"

namespace server::ai::utility
{

float getColliderRadius(ecs::World &world, ecs::Entity entity, float defaultRadius)
{
  if (!world.hasComponent<ecs::Collider>(entity)) {
    return defaultRadius;
  }

  const auto &col = world.getComponent<ecs::Collider>(entity);

  if (col.shape == ecs::Collider::Shape::BOX) {
    return std::max(col.width, col.height) * 0.5f;
  }

  // Circle
  return col.radius;
}

void predictEntityPosition(ecs::World &world, ecs::Entity entity, float predictionTime, float &outX, float &outY)
{
  if (!world.hasComponent<ecs::Transform>(entity)) {
    outX = 0.0f;
    outY = 0.0f;
    return;
  }

  auto &transform = world.getComponent<ecs::Transform>(entity);
  outX = transform.x;
  outY = transform.y;

  if (world.hasComponent<ecs::Velocity>(entity)) {
    const auto &vel = world.getComponent<ecs::Velocity>(entity);
    outX += vel.dx * predictionTime;
    outY += vel.dy * predictionTime;
  }
}

bool isEntityValid(ecs::World &world, ecs::Entity entity)
{
  return world.isAlive(entity) && world.hasComponent<ecs::Transform>(entity) &&
         world.hasComponent<ecs::Velocity>(entity);
}

} // namespace server::ai::utility
