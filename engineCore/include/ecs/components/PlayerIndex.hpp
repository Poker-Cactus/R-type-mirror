/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PlayerIndex.hpp - Component for player sprite sheet row selection
*/

#ifndef ECS_COMPONENTS_PLAYERINDEX_HPP_
#define ECS_COMPONENTS_PLAYERINDEX_HPP_

#include "IComponent.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief PlayerIndex component - specifies which player sprite row to use
 *
 * DESIGN PRINCIPLE:
 * The server assigns a player index (0-4) which maps to sprite sheet rows.
 * The client uses this index to select the correct row from players_ships.gif.
 *
 * Sprite sheet layout:
 * - 5 rows (one per player index)
 * - 5 columns (animation frames: down, down-trans, neutral, up-trans, up)
 * - Image size: 166x86 pixels
 */
struct PlayerIndex : public IComponent {
  std::uint32_t index = 0; // Player index (0-4)

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"index", index}};
  }

  static PlayerIndex fromJson(const nlohmann::json &json)
  {
    PlayerIndex pi;
    pi.index = json.value("index", 0u);
    return pi;
  }
};

} // namespace ecs

#endif // ECS_COMPONENTS_PLAYERINDEX_HPP_
