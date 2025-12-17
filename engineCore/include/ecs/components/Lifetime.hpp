/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lifetime.hpp - Generic lifetime timer for entities
*/

#ifndef ECS_COMPONENTS_LIFETIME_HPP_
#define ECS_COMPONENTS_LIFETIME_HPP_

namespace ecs
{

struct Lifetime {
  float remaining = 0.0F;
};

} // namespace ecs

#endif // ECS_COMPONENTS_LIFETIME_HPP_
