/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Owner component - tracks entity ownership
*/

#ifndef ECS_OWNER_HPP_
#define ECS_OWNER_HPP_

#include "../Entity.hpp"

namespace ecs
{

/**
 * @brief Owner component - tracks who owns/spawned this entity
 * Used for projectiles to prevent self-damage
 */
struct Owner {
  Entity ownerId = 0;
};

} // namespace ecs

#endif // ECS_OWNER_HPP_
