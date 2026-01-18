/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Ally.hpp - Component for ally-controlled entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ALLY_HPP
#define ENGINECORE_ECS_COMPONENTS_ALLY_HPP

#include "../../../server/include/ai/AllyAIUtility.hpp"
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
  server::ai::AIStrength strength;

  Ally() : strength(server::ai::AIStrength::MEDIUM) {}
  Ally(server::ai::AIStrength s) : strength(s) {}

  [[nodiscard]] nlohmann::json toJson() const override { return {{"strength", static_cast<int>(strength)}}; }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ALLY_HPP
