/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Immortal.hpp - Marks entities as immortal (cannot take damage)
*/

#ifndef ECS_COMPONENTS_IMMORTAL_HPP_
#define ECS_COMPONENTS_IMMORTAL_HPP_
namespace ecs
{

struct Immortal {
    bool isImmortal = true;
};

} // namespace ecs

#endif // ECS_COMPONENTS_IMMORTAL_HPP_