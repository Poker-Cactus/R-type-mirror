/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry implementation
*/

#include "ecs/Registry.hpp"

namespace ECS
{

std::size_t Registry::createEntity()
{
    return _nextEntityId++;
}

} // namespace ECS
