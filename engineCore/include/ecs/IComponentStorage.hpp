/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IComponentStorage.hpp
*/

#ifndef ECS_ICOMPONENTSTORAGE_HPP_
#define ECS_ICOMPONENTSTORAGE_HPP_

#include "Entity.hpp"

class IComponentStorage
{
public:
  virtual ~IComponentStorage() = default;
  virtual void removeComponent(ecs::Entity ent) = 0;
  [[nodiscard]] virtual bool hasComponent(ecs::Entity ent) const = 0;
};

#endif // ECS_ICOMPONENTSTORAGE_HPP_