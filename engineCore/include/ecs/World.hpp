/*
** EPITECH PROJECT, 2025
** rtype (Workspace)
** File description:
** World.hpp
*/

#ifndef ECS_WORLD_HPP_
#define ECS_WORLD_HPP_

#include "ComponentManager.hpp"
#include "ComponentSignature.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"
#include <cstddef>
#include <vector>

namespace ecs
{
/**
 * @brief Central coordinator for the ECS architecture
 *
 * The World manages entities, systems and components, providing a unified interface
 * for game logic. It orchestrates the update cycle and provides access to
 * the EntityManager, SystemManager and ComponentManager.
 *
 * @note This is typically instantiated once per game/scene
 *
 * @example
 * World world;
 * Entity player = world.createEntity();
 * world.addComponent(player, Position{0.0f, 0.0f});
 *
 * world.registerSystem<PhysicsSystem>();
 * world.registerSystem<RenderSystem>();
 *
 * // Game loop
 * while (running) {
 *     world.update(deltaTime);
 * }
 */
class World
{
public:
  World() = default;
  ~World() = default;

  // ========== Entity Management ==========

  [[nodiscard]] Entity createEntity()
  {
    Entity entity = m_entityManager.createEntity();
    return entity;
  }

  void destroyEntity(Entity entity)
  {
    if (!m_entityManager.isAlive(entity)) {
      return;
    }

    m_componentManager.removeAllComponents(entity);
    m_systemManager.onEntityDestroyed(entity);
    m_entityManager.destroyEntity(entity);
  }

  [[nodiscard]] bool isAlive(Entity entity) const { return m_entityManager.isAlive(entity); }

  [[nodiscard]] std::size_t getEntityCount() const { return m_entityManager.getAliveCount(); }

  // ========== System Management ==========

  template <typename T, typename... Args>
  T &registerSystem(Args &&...args)
  {
    return m_systemManager.registerSystem<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  T *getSystem()
  {
    return m_systemManager.getSystem<T>();
  }

  template <typename T>
  [[nodiscard]] const T *getSystem() const
  {
    return m_systemManager.getSystem<T>();
  }

  template <typename T>
  [[nodiscard]] bool hasSystem() const noexcept
  {
    return m_systemManager.hasSystem<T>();
  }

  template <typename T>
  void removeSystem() noexcept
  {
    m_systemManager.removeSystem<T>();
  }

  void update(float deltaTime) { m_systemManager.update(*this, deltaTime); }

  [[nodiscard]] std::size_t getSystemCount() const noexcept { return m_systemManager.getSystemCount(); }

  void clearSystems() noexcept { m_systemManager.clear(); }

  // ========== Component Management ==========

  template <typename T>
  void addComponent(Entity entity, T component)
  {
    m_componentManager.addComponent(entity, std::move(component));

    // Update signature in EntityManager (single source of truth)
    ComponentSignature signature = m_entityManager.getSignature(entity);
    signature.set(ecs::getComponentId<T>());
    m_entityManager.setSignature(entity, signature);

    // Notify systems
    m_systemManager.onEntitySignatureChanged(entity, signature);
  }

  template <typename T>
  T &getComponent(Entity entity)
  {
    return m_componentManager.getComponent<T>(entity);
  }

  template <typename T>
  [[nodiscard]] const T &getComponent(Entity entity) const
  {
    return m_componentManager.getComponent<T>(entity);
  }

  template <typename T>
  [[nodiscard]] bool hasComponent(Entity entity) const
  {
    return m_componentManager.hasComponent<T>(entity);
  }

  template <typename T>
  void removeComponent(Entity entity)
  {
    m_componentManager.removeComponent<T>(entity);

    // Update signature in EntityManager (single source of truth)
    ComponentSignature signature = m_entityManager.getSignature(entity);
    signature.reset(ecs::getComponentId<T>());
    m_entityManager.setSignature(entity, signature);

    // Notify systems
    m_systemManager.onEntitySignatureChanged(entity, signature);
  }

  void removeAllComponents(Entity entity)
  {
    m_componentManager.removeAllComponents(entity);

    // Reset signature in EntityManager (single source of truth)
    ComponentSignature emptySignature;
    m_entityManager.setSignature(entity, emptySignature);

    // Notify systems
    m_systemManager.onEntitySignatureChanged(entity, emptySignature);
  }

  [[nodiscard]] const ComponentSignature &getEntitySignature(Entity entity) const
  {
    return m_entityManager.getSignature(entity);
  }

  /**
   * @brief Filters entities by component signature (bitwise matching)
   */
  void getEntitiesWithSignature(const ComponentSignature &signature, std::vector<Entity> &entities) const
  {
    entities.clear();

    for (Entity entity = 0; entity < m_entityManager.getTotalCount(); ++entity) {
      if (!m_entityManager.isAlive(entity)) {
        continue;
      }

      const ComponentSignature &entitySignature = m_entityManager.getSignature(entity);

      if ((entitySignature & signature) == signature) {
        entities.push_back(entity);
      }
    }
  }

private:
  EntityManager m_entityManager;
  ComponentManager m_componentManager;
  SystemManager m_systemManager;
};

} // namespace ecs

#endif // ECS_WORLD_HPP_
