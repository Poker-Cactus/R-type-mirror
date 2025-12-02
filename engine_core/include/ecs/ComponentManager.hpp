/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ComponentManager.hpp
*/

#ifndef ECS_COMPONENTMANAGER_HPP_
#define ECS_COMPONENTMANAGER_HPP_

#include "ComponentStorage.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>

class ComponentManager
{
public:
  ~ComponentManager() = default;

  template <typename T>
  ComponentStorage<T> &ensureStorage()
  {
    auto key = std::type_index(typeid(T));
    auto iterator = storages.find(key);

    if (iterator != storages.end()) {
      return static_cast<ComponentStorage<T> &>(*iterator->second);
    }

    // Si le storage n'existe pas, on le crée
    auto newStorage = std::make_unique<ComponentStorage<T>>();
    ComponentStorage<T> &ref = *newStorage;
    storages.emplace(key, std::move(newStorage));
    return ref;
  }

  template <typename T>
  void addComponent(Entity ent, T component)
  {
    ComponentStorage<T> &storage = ensureStorage<T>();
    storage.addComponent(ent, component);
  }

  template <typename T>
  T &getComponent(Entity ent)
  {
    ComponentStorage<T> &storage = ensureStorage<T>();
    return storage.getComponent(ent);
  }

  template <typename T>
  const T &getComponent(Entity ent) const
  {
    auto key = std::type_index(typeid(T));
    auto iterator = storages.find(key);
    if (iterator != storages.end()) {
      return static_cast<const ComponentStorage<T> &>(*iterator->second).getComponent(ent);
    }
    throw std::out_of_range("No such component storage");
  }

  template <typename T>
  bool hasComponent(Entity ent) const
  {
    auto key = std::type_index(typeid(T));
    auto iterator = storages.find(key);

    if (iterator != storages.end()) {
      return iterator->second->hasComponent(ent);
    }
    return false;
  }

  template <typename T>
  void removeComponent(Entity ent)
  {
    auto key = std::type_index(typeid(T));
    auto iterator = storages.find(key);

    if (iterator != storages.end()) {
      iterator->second->removeComponent(ent);
    }
  }

  void removeAllComponents(Entity ent)
  {
    for (auto &pair : storages) {
      pair.second->removeComponent(ent);
    }
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
};

#endif // ECS_COMPONENTMANAGER_HPP_