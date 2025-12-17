/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Networked
*/

#ifndef NETWORKED_HPP_
#define NETWORKED_HPP_

#include "IComponent.hpp"

namespace ecs
{
struct Networked : public IComponent {
  Entity networkId;

  [[nodiscard]] nlohmann::json toJson() const { return {{"networkId", networkId}}; }
};
} // namespace ecs

#endif /* !NETWORKED_HPP_ */
