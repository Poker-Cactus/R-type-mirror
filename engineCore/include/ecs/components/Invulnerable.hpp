/*
** EPITECH PROJECT, 2025
** R-type
** File description:
** Invulnerable.hpp - Temporary invulnerability component
*/

#ifndef ECS_COMPONENTS_INVULNERABLE_HPP_
#define ECS_COMPONENTS_INVULNERABLE_HPP_

namespace ecs
{

struct Invulnerable {
  float remaining = 0.0F; // seconds of remaining invulnerability
};

} // namespace ecs

#endif // ECS_COMPONENTS_INVULNERABLE_HPP_
