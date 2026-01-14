/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PlayerAnimationSystem.hpp - Server-side player ship animation
*/

#ifndef SERVER_PLAYER_ANIMATION_SYSTEM_HPP_
#define SERVER_PLAYER_ANIMATION_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/PlayerIndex.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "ecs/ComponentSignature.hpp"
#include <iostream>
#include <vector>

namespace server
{

/**
 * @brief Server-side system that animates player ships based on input
 *
 * DESIGN PRINCIPLE:
 * Server reads Input component and sets sprite.currentFrame accordingly.
 * Client renders whatever frame the server sends (network-authoritative).
 *
 * Animation frames (vertical):
 * - Frame 0: Max down tilt
 * - Frame 1: Down tilt
 * - Frame 2: Neutral (no input)
 * - Frame 3: Up tilt
 * - Frame 4: Max up tilt
 */
class PlayerAnimationSystem : public ecs::ISystem
{
public:
  PlayerAnimationSystem() { std::cout << "[PlayerAnimationSystem] Initialized" << std::endl; }

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    // Debug: log entity count (only first few times)
    static int updateCount = 0;
    if (updateCount < 5) {
      std::cout << "[PlayerAnimationSystem] Found " << entities.size() << " player entities" << std::endl;
      updateCount++;
    }

    for (auto entity : entities) {
      const auto &input = world.getComponent<ecs::Input>(entity);
      auto &sprite = world.getComponent<ecs::Sprite>(entity);

      // Store previous frame for debug
      auto prevFrame = sprite.currentFrame;

      // Determine frame based on input (instant response, no interpolation)
      if (input.up && !input.down) {
        // Moving up
        sprite.currentFrame = 4; // Max up tilt
      } else if (input.down && !input.up) {
        // Moving down
        sprite.currentFrame = 0; // Max down tilt
      } else {
        // Neutral (no vertical input or both pressed)
        sprite.currentFrame = 2;
      }

      // Debug log when frame changes OR when there's input
      if (prevFrame != sprite.currentFrame || input.up || input.down) {
        std::cout << "[PlayerAnimationSystem] Entity " << entity << " frame: " << prevFrame << " -> "
                  << sprite.currentFrame << " (up=" << input.up << ", down=" << input.down << ")" << std::endl;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Input>());
    sig.set(ecs::getComponentId<ecs::Sprite>());
    sig.set(ecs::getComponentId<ecs::PlayerIndex>()); // Only players
    return sig;
  }
};

} // namespace server

#endif // SERVER_PLAYER_ANIMATION_SYSTEM_HPP_
