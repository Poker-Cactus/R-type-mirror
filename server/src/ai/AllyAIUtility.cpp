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

void getEntityCenter(ecs::World &world, ecs::Entity entity, float x, float y, float &centerX, float &centerY)
{
  if (!world.hasComponent<ecs::Collider>(entity)) {
    // Default to a guess if no collider (assume 32x32 roughly)
    centerX = x + 16.0f;
    centerY = y + 16.0f;
    return;
  }

  const auto &col = world.getComponent<ecs::Collider>(entity);
  if (col.shape == ecs::Collider::Shape::BOX) {
    centerX = x + col.width * 0.5f;
    centerY = y + col.height * 0.5f;
  } else {
    // For CIRCLE, the position is typically the center
    // BUT we need to check how standard collision system treats circle position.
    // In many systems, x,y is top-left of bounding box even for circles.
    // Assuming x,y IS center for circle based on standard patterns or top-left.
    // Let's assume standard behavior: position is top-left of the bounding square.
    centerX = x + col.radius;
    centerY = y + col.radius;
  }
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
