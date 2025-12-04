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
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

class ComponentManager
{
public:
  ~ComponentManager() = default;

  template <typename T>
  void addComponent(Entity ent, const T &component)
  {
    ComponentStorage<T> &storage = ensureStorage<T>();
    storage.addComponent(ent, component);

    // Update entity signature
    entitySignatures[ent].set(ecs::getComponentId<T>());
  }

  template <typename T>
  T &getComponent(Entity ent)
  {
    ComponentStorage<T> &storage = ensureStorage<T>();
    return storage.getComponent(ent);
  }

  template <typename T>
  [[nodiscard]] [[nodiscard]] const T &getComponent(Entity ent) const
  {
    auto key = std::type_index(typeid(T));
    auto iterator = storages.find(key);
    if (iterator != storages.end()) {
      return static_cast<const ComponentStorage<T> &>(*iterator->second).getComponent(ent);
    }
    throw std::out_of_range("No such component storage");
  }

  template <typename T>
  [[nodiscard]] [[nodiscard]] [[nodiscard]] [[nodiscard]] [[nodiscard]] bool hasComponent(Entity ent) const
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

      // Update entity signature
      entitySignatures[ent].reset(ecs::getComponentId<T>());
    }
  }

  void removeAllComponents(Entity ent)
  {
    for (auto &pair : storages) {
      pair.second->removeComponent(ent);
    }

    // Clear entity signature
    entitySignatures[ent].reset();
  }

  /**
   * @brief Gets the component signature for an entity
   * @param ent Entity to get signature for
   * @return Component signature bitset for the entity
   */
  [[nodiscard]] const ecs::ComponentSignature &getEntitySignature(Entity ent) const
  {
    static const ecs::ComponentSignature emptySignature;
    auto iter = entitySignatures.find(ent);
    if (iter != entitySignatures.end()) {
      return iter->second;
    }
    return emptySignature;
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
  std::unordered_map<Entity, ecs::ComponentSignature> entitySignatures;

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
};

#endif // ECS_COMPONENTMANAGER_HPP_