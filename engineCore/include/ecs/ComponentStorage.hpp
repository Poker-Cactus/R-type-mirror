/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ComponentStorage.hpp
*/

#ifndef ECS_COMPONENTSTORAGE_HPP_
#define ECS_COMPONENTSTORAGE_HPP_

#include "Entity.hpp"
#include "IComponentStorage.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

template <typename T>
class ComponentStorage : public IComponentStorage
{
public:
  void addComponent(Entity ent, const T &component)
  {
    if (ent >= sparseArray.size()) {
      sparseArray.resize(ent + 1, INVALID);
    }
    if (sparseArray[ent] != INVALID) {
      const std::size_t index = sparseArray[ent];
      denseComponentArray[index] = component;
    } else {
      denseEntityArray.push_back(ent);
      denseComponentArray.push_back(component);
      sparseArray[ent] = denseEntityArray.size() - 1;
    }
  }

  void removeComponent(Entity ent) override
  {
    if (ent >= sparseArray.size() || sparseArray[ent] == INVALID) {
      return;
    }

    std::size_t denseIndex = sparseArray[ent];

    // recuperer le dernier element pour le mettre a la place de celui qu'on supp
    std::size_t lastIndex = denseComponentArray.size() - 1;
    Entity lastEntity = denseEntityArray[lastIndex];
    auto lastComponent = denseComponentArray[lastIndex];

    // deplacer le dernier a la place de ent si c'est pas le dernier
    if (denseIndex != lastIndex) {
      denseEntityArray[denseIndex] = lastEntity;
      denseComponentArray[denseIndex] = lastComponent;
      sparseArray[lastEntity] = denseIndex;
    }
    // supprimer le dernier
    denseEntityArray.pop_back();
    denseComponentArray.pop_back();
    sparseArray[ent] = INVALID;
  }

  [[nodiscard]] bool hasComponent(Entity ent) const override
  {
    return ent < sparseArray.size() && sparseArray[ent] != INVALID;
  }

  T &getComponent(Entity ent)
  {
    if (!hasComponent(ent)) {
      throw std::out_of_range("Entity does not have this component");
    }
    return denseComponentArray[sparseArray[ent]];
  }

  [[nodiscard]] const T &getComponent(Entity ent) const
  {
    if (!hasComponent(ent)) {
      throw std::out_of_range("Entity does not have this component");
    }
    return denseComponentArray[sparseArray[ent]];
  }

private:
  static constexpr std::size_t INVALID = std::numeric_limits<std::size_t>::max();

  std::vector<std::size_t> sparseArray;
  std::vector<Entity> denseEntityArray;
  std::vector<T> denseComponentArray;
};

#endif
