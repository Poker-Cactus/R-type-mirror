/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ScoreSystem.hpp - Handles score tracking
*/

#ifndef SERVER_SCORE_SYSTEM_HPP_
#define SERVER_SCORE_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Score.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <iostream>

namespace server
{

/**
 * @brief System that tracks score from game events
 */
class ScoreSystem : public ecs::ISystem
{
public:
  ScoreSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;
    (void)world;
    // This system works via event subscription
  }

  void initialize(ecs::World &world)
  {
    m_scoreHandle =
      world.subscribeEvent<ecs::ScoreEvent>([&world](const ecs::ScoreEvent &event) { handleScore(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

private:
  ecs::EventListenerHandle m_scoreHandle;

  static void handleScore(ecs::World &world, const ecs::ScoreEvent &event)
  {
    // Add score to the player entity
    if (world.isAlive(event.player) && world.hasComponent<ecs::Score>(event.player)) {
      auto &score = world.getComponent<ecs::Score>(event.player);
      score.points += event.points;
      std::cout << "[ScoreSystem] Added " << event.points << " points to entity " << event.player
                << " (total: " << score.points << ")" << std::endl;
    } else {
      std::cout << "[ScoreSystem] Cannot add score - entity " << event.player
                << " is not alive or has no Score component" << std::endl;
    }
  }
};

} // namespace server

#endif // SERVER_SCORE_SYSTEM_HPP_
