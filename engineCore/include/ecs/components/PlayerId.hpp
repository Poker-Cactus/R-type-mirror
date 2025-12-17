/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PlayerId.hpp - Marks an entity as controlled by a given clientId
*/

#ifndef ECS_COMPONENTS_PLAYERID_HPP_
#define ECS_COMPONENTS_PLAYERID_HPP_

#include <cstdint>

namespace ecs
{

struct PlayerId {
  std::uint32_t clientId = 0;
};

} // namespace ecs

#endif // ECS_COMPONENTS_PLAYERID_HPP_
