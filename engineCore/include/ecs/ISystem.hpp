/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** System.hpp
*/

#ifndef ECS_ISYSTEM_HPP_
#define ECS_ISYSTEM_HPP_

#include "ComponentSignature.hpp"

namespace ecs
{
class World;

/**
 * @brief Interface for all systems in the ECS architecture
 *
 * Systems contain the logic that operates on entities with specific components.
 * Each system implements the update() method which is called every frame.
 *
 * @note Systems should be stateless when possible, storing only configuration
 * @note Use World::getComponentManager() to access entity components
 *
 * @example
 * class PhysicsSystem : public ISystem {
 * public:
 *     void update(World& world, float deltaTime) override {
 *         // Update physics for all entities
 *     }
 * };
 */
class ISystem
{
public:
  virtual ~ISystem() = default;

  /**
   * @brief Updates the system logic
   * @param world Reference to the world containing entities and components
   * @param deltaTime Time elapsed since last update (in seconds)
   *
   * This method is called every frame by the SystemManager.
   * Implement your system's logic here (physics, rendering, AI, etc.)
   */
  virtual void update(World &world, float deltaTime) = 0;

  /**
   * @brief Gets the component signature required by this system
   * @return Component signature bitset indicating required components
   *
   * The signature defines which components an entity must have for this
   * system to operate on it. This enables efficient entity filtering
   * using bitwise operations (O(1) complexity).
   *
   * @example
   * class PhysicsSystem : public ISystem {
   *     ComponentSignature getSignature() const override {
   *         ComponentSignature sig;
   *         sig.set(getComponentId<Position>());
   *         sig.set(getComponentId<Velocity>());
   *         return sig; // Requires Position AND Velocity
   *     }
   * };
   */
  [[nodiscard]] virtual ComponentSignature getSignature() const = 0;
};
} // namespace ecs

#endif // ECS_ISYSTEM_HPP_
