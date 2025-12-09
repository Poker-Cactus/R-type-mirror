/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IComponent.hpp
*/

#ifndef ECS_ICOMPONENT_HPP_
#define ECS_ICOMPONENT_HPP_

#include <nlohmann/json.hpp>

namespace ecs
{

class IComponent
{
public:
  virtual ~IComponent() = default;

  /**
   * @brief Convert component data into JSON
   */
  [[nodiscard]] virtual nlohmann::json toJson() const = 0;
};

} // namespace ecs

#endif // ECS_ICOMPONENT_HPP_
