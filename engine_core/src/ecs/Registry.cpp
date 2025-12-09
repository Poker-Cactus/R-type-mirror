/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry.cpp
*/

#include "../../include/ecs/Registry.hpp"

namespace rtype::ecs
{

std::size_t Registry::createEntity()
{
    return m_nextEntityId++;
}

} // namespace rtype::ecs
