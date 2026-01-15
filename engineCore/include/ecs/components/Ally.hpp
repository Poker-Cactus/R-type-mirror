/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Ally.hpp - Component for ally-controlled entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ALLY_HPP
#define ENGINECORE_ECS_COMPONENTS_ALLY_HPP

#include "IComponent.hpp"

namespace ecs
{
/**
 * @brief Component that marks an entity as ally-controlled
 *
 * This component is used to identify entities that are controlled by ally systems
 * rather than player input. It can contain ally-specific state information.
 */
struct Ally : public IComponent {
  // Ally state or parameters can be added here
  // For now, it's just a marker component

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {}; // Empty for now, can add fields later
  }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_AI_HPP
