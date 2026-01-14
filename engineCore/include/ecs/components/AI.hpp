/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AI.hpp - Component for AI-controlled entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_AI_HPP
#define ENGINECORE_ECS_COMPONENTS_AI_HPP

#include "IComponent.hpp"

namespace ecs
{
/**
 * @brief Component that marks an entity as AI-controlled
 *
 * This component is used to identify entities that are controlled by AI systems
 * rather than player input. It can contain AI-specific state information.
 */
struct AI : public IComponent {
  // AI state or parameters can be added here
  // For now, it's just a marker component

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {}; // Empty for now, can add fields later
  }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_AI_HPP
