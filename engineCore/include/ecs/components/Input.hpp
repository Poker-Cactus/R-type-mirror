/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Input.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_INPUT_HPP
#define ENGINECORE_ECS_COMPONENTS_INPUT_HPP

#include "IComponent.hpp"

namespace ecs
{
struct Input : public IComponent {
  bool up;
  bool down;
  bool left;
  bool right;
  bool shoot;
  bool chargedShoot;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"up", up}, {"down", down}, {"left", left}, {"right", right}, {"shoot", shoot}, {"chargedShoot", chargedShoot}};
  }
};
} // namespace ecs
#endif // ENGINECORE_ECS_COMPONENTS_INPUT_HPP
