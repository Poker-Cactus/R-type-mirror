/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ComponentManager.hpp
*/

#ifndef ECS_COMPONENTMANAGER_HPP_
#define ECS_COMPONENTMANAGER_HPP_

#include "ComponentStorage.hpp"
#include "IComponentStorage.hpp"
#include "ecs/Entity.hpp"
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

class ComponentManager
{
public:
  ~ComponentManager() = default;

  // ========= ADD =========
  template <typename T>
  void addComponent(ecs::Entity ent, const T &component)
  {
    auto &storage = ensureStorage<T>();
    storage.addComponent(ent, component);
  }

  // ========= GET =========
  template <typename T>
  T &getComponent(ecs::Entity ent)
  {
    auto &storage = ensureStorage<T>();
    return storage.getComponent(ent);
  }

  template <typename T>
  const T &getComponent(ecs::Entity ent) const
  {
    auto key = std::type_index(typeid(T));
    auto iter = storages.find(key);

    if (iter == storages.end()) {
      throw std::out_of_range("Component not registered");
    }

    return static_cast<const ComponentStorage<T> &>(*iter->second).getComponent(ent);
  }

  // ========= HAS =========
  template <typename T>
  bool hasComponent(ecs::Entity ent) const
  {
    auto key = std::type_index(typeid(T));
    auto iter = storages.find(key);

    if (iter == storages.end()) {
      return false;
    }

    return iter->second->hasComponent(ent);
  }

  // ========= REMOVE =========
  template <typename T>
  void removeComponent(ecs::Entity ent)
  {
    auto key = std::type_index(typeid(T));
    auto iter = storages.find(key);

    if (iter == storages.end()) {
      return;
    }

    iter->second->removeComponent(ent);
  }

  // ========= REMOVE ALL =========
  void removeAllComponents(ecs::Entity ent)
  {
    for (auto &pair : storages) {
      pair.second->removeComponent(ent);
    }
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;

  template <typename T>
  ComponentStorage<T> &ensureStorage()
  {
    auto key = std::type_index(typeid(T));
    auto iter = storages.find(key);

    if (iter != storages.end()) {
      return static_cast<ComponentStorage<T> &>(*iter->second);
    }

    auto storage = std::make_unique<ComponentStorage<T>>();
    auto &ref = *storage;
    storages.emplace(key, std::move(storage));
    return ref;
  }
};

#endif // ECS_COMPONENTMANAGER_HPP_
