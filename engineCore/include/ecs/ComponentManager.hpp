/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ComponentManager.hpp
*/

#ifndef ECS_COMPONENTMANAGER_HPP_
#define ECS_COMPONENTMANAGER_HPP_

#include "ComponentSignature.hpp"
#include "ComponentStorage.hpp"
#include "ecs/IComponentStorage.hpp"
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
  void addComponent(Entity ent, const T &component)
  {
    auto &storage = ensureStorage<T>();
    storage.addComponent(ent, component);

    entitySignatures[ent].set(ecs::getComponentId<T>());
  }

  // ========= GET =========
  template <typename T>
  T &getComponent(Entity ent)
  {
    auto &storage = ensureStorage<T>();
    return storage.getComponent(ent);
  }

  template <typename T>
  const T &getComponent(Entity ent) const
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
  bool hasComponent(Entity ent) const
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
  void removeComponent(Entity ent)
  {
    auto key = std::type_index(typeid(T));
    auto iter = storages.find(key);

    if (iter == storages.end()) {
      return;
    }

    iter->second->removeComponent(ent);
    entitySignatures[ent].reset(ecs::getComponentId<T>());
  }

  // ========= REMOVE ALL =========
  void removeAllComponents(Entity ent)
  {
    for (auto &pair : storages) {
      pair.second->removeComponent(ent);
    }

    entitySignatures[ent].reset();
  }

  // ========= SIGNATURE =========
  const ecs::ComponentSignature &getEntitySignature(Entity ent) const
  {
    static const ecs::ComponentSignature empty{};
    auto iter = entitySignatures.find(ent);
    return (iter != entitySignatures.end()) ? iter->second : empty;
  }

  void setEntitySignature(Entity ent, const ecs::ComponentSignature &sig) { entitySignatures[ent] = sig; }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
  std::unordered_map<Entity, ecs::ComponentSignature> entitySignatures;

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
