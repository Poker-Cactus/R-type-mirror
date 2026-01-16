/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAI.cpp - Implementation of main ally AI controller
*/

#include "../../include/ai/AllyAI.hpp"
#include "../../include/ai/AllyAIUtility.hpp"
#include "../../include/ai/AllyBehavior.hpp"
#include "../../include/ai/AllyPerception.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"

namespace server::ai
{

AllyAI::AllyAI()
{
}

void AllyAI::update(ecs::World &world, ecs::Entity allyEntity, float deltaTime)
{
  // Validate entity
  if (!world.isAlive(allyEntity) || !utility::isEntityValid(world, allyEntity)) {
    return;
  }

  // Only process in solo mode (single player)
  std::vector<ecs::Entity> players;
  ecs::ComponentSignature playerSig;
  playerSig.set(ecs::getComponentId<ecs::PlayerId>());
  world.getEntitiesWithSignature(playerSig, players);

  if (players.size() != 1) {
    return; // Not solo mode
  }

  ecs::Entity playerEntity = players[0];

  // Perform all AI updates
  updateBehaviors(world, allyEntity, deltaTime);

  // Apply viewport constraints at the end
  perception::ViewportConstraint::constrainToViewport(world, allyEntity, playerEntity);
}

void AllyAI::reset()
{
  m_movement.reset();
  m_shooting.reset();
  m_avoidance.reset();
}

void AllyAI::updateBehaviors(ecs::World &world, ecs::Entity allyEntity, float deltaTime)
{
  // Get ally's current state
  auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
  auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);

  // Find player for reference
  std::vector<ecs::Entity> players;
  ecs::ComponentSignature playerSig;
  playerSig.set(ecs::getComponentId<ecs::PlayerId>());
  world.getEntitiesWithSignature(playerSig, players);

  if (players.empty()) {
    return;
  }

  ecs::Entity playerEntity = players[0];
  auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

  // STEP 1: Detect nearest enemy
  ecs::Entity targetEntity = perception::EnemyPerception::findNearestEnemy(world, allyTransform.x, allyTransform.y, playerEntity);

  // Use enemy as target, or player if no enemy
  const ecs::Transform &targetTransform = (targetEntity != 0) ? world.getComponent<ecs::Transform>(targetEntity)
                                                               : playerTransform;

  // STEP 2: Update movement toward target
  m_movement.update(deltaTime, allyVelocity, allyTransform, targetTransform);

  // STEP 3: Update shooting (only if enemy detected)
  if (targetEntity != 0) {
    m_shooting.update(deltaTime, world, allyEntity, allyTransform, targetTransform);
  }

  // STEP 4: Apply obstacle avoidance (highest priority)
  m_avoidance.update(world, allyEntity, allyVelocity, allyTransform);

  // STEP 5: Update animation based on final velocity
  m_animation.update(world, allyEntity, allyVelocity);
}

} // namespace server::ai
