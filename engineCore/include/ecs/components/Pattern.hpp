/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Pattern.hpp - Component for entity movement patterns
*/

#ifndef ENGINECORE_ECS_COMPONENTS_PATTERN_HPP
#define ENGINECORE_ECS_COMPONENTS_PATTERN_HPP

#include "IComponent.hpp"
#include <string>

namespace ecs
{
/**
 * @brief Defines the movement pattern of an entity
 *
 * This component is used by the MovementSystem to apply
 * different movement behaviors to entities (enemies, powerups, etc.)
 */
struct Pattern : public IComponent {
  std::string patternType; // e.g., "sine_wave", "straight", "zigzag", "circle"
  float amplitude; // Amplitude for wave patterns
  float frequency; // Frequency for oscillating patterns
  float phase; // Current phase in the pattern cycle

  Pattern() : patternType("straight"), amplitude(0.0f), frequency(0.0f), phase(0.0f) {}

  Pattern(std::string type, float amp = 0.0f, float freq = 0.0f)
      : patternType(std::move(type)), amplitude(amp), frequency(freq), phase(0.0f)
  {
  }

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"patternType", patternType}, {"amplitude", amplitude}, {"frequency", frequency}, {"phase", phase}};
  }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_PATTERN_HPP
