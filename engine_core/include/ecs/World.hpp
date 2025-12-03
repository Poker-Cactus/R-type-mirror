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
#include "SystemManager.hpp"
#include <vector>

namespace ecs
{
/**
 * @brief Central coordinator for the ECS architecture
 *
 * The World manages both systems and components, providing a unified interface
 * for game logic. It orchestrates the update cycle and provides access to
 * both the SystemManager and ComponentManager.
 *
 * @note This is typically instantiated once per game/scene
 *
 * @example
 * World world;
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

  /**
   * @brief Registers a system in the world
   * @tparam T System type (must inherit from ISystem)
   * @tparam Args Constructor argument types
   * @param args Arguments forwarded to T's constructor
   * @return Reference to the registered system
   */
  template <typename T, typename... Args>
  T &registerSystem(Args &&...args)
  {
    return systemManager.registerSystem<T>(std::forward<Args>(args)...);
  }

  /**
   * @brief Retrieves a non-const pointer to a registered system
   * @tparam T System type to retrieve
   * @return Pointer to the system, or nullptr if not found
   */
  template <typename T>
  T *getSystem()
  {
    return systemManager.getSystem<T>();
  }

  /**
   * @brief Retrieves a const pointer to a registered system
   * @tparam T System type to retrieve
   * @return Const pointer to the system, or nullptr if not found
   */
  template <typename T>
  const T *getSystem() const
  {
    return systemManager.getSystem<T>();
  }

  /**
   * @brief Checks if a system is registered
   * @tparam T System type to check
   * @return true if system is registered, false otherwise
   */
  template <typename T>
  [[nodiscard]] bool hasSystem() const noexcept
  {
    return systemManager.hasSystem<T>();
  }

  /**
   * @brief Removes a system from the world
   * @tparam T System type to remove
   */
  template <typename T>
  void removeSystem() noexcept
  {
    systemManager.removeSystem<T>();
  }

  /**
   * @brief Updates all registered systems
   * @param deltaTime Time elapsed since last update (in seconds)
   */
  void update(float deltaTime) { systemManager.update(*this, deltaTime); }

  /**
   * @brief Gets the component manager for entity component access
   * @return Reference to the component manager
   */
  ComponentManager &getComponentManager() { return componentManager; }

  /**
   * @brief Gets the component manager (const version)
   * @return Const reference to the component manager
   */
  [[nodiscard]] const ComponentManager &getComponentManager() const { return componentManager; }

  /**
   * @brief Gets the component signature for an entity
   * @param entity Entity to get signature for
   * @return Component signature bitset for the entity
   */
  [[nodiscard]] const ComponentSignature &getEntitySignature(Entity entity) const
  {
    return componentManager.getEntitySignature(entity);
  }

  /**
   * @brief Filters entities by component signature (bitwise matching)
   * @param signature Required component signature
   * @param entities Vector to store matching entity IDs
   *
   * This performs fast bitwise AND operations to find entities that have
   * all components specified in the signature. This is O(n) where n is the
   * number of entities, but each check is a single CPU operation.
   *
   * @example
   * ComponentSignature physicsSig;
   * physicsSig.set(getComponentId<Position>());
   * physicsSig.set(getComponentId<Velocity>());
   *
   * std::vector<Entity> physicsEntities;
   * world.getEntitiesWithSignature(physicsSig, physicsEntities);
   * // physicsEntities now contains all entities with Position AND Velocity
   */
  void getEntitiesWithSignature(const ComponentSignature &signature, std::vector<Entity> &entities) const;

  /**
   * @brief Returns the number of registered systems
   * @return Number of systems in the world
   */
  [[nodiscard]] std::size_t getSystemCount() const noexcept { return systemManager.getSystemCount(); }

  /**
   * @brief Removes all systems from the world
   */
  void clearSystems() noexcept { systemManager.clear(); }

private:
  SystemManager systemManager;
  ComponentManager componentManager;
};
} // namespace ecs

#endif // ECS_WORLD_HPP_
