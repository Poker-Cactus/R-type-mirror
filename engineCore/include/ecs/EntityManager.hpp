/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EntityManager.hpp
*/

#ifndef ECS_ENTITYMANAGER_HPP_
#define ECS_ENTITYMANAGER_HPP_

#include "ComponentSignature.hpp"
#include "Entity.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace ecs
{
/**
 * @brief Manages entity lifecycle and signatures
 *
 * The EntityManager is responsible for creating and destroying entities,
 * managing their signatures (component masks), and tracking entity states.
 * It implements entity ID recycling for efficient memory usage.
 *
 * @note Maximum entities is defined by MAX_ENTITIES constant
 */
class EntityManager
{
public:
  /**
   * @brief Constructs the EntityManager
   * Initializes entity tracking and pre-allocates signature storage
   */
  EntityManager() : m_nextId(0), m_livingEntityCount(0) { m_signatures.resize(MAX_ENTITIES); }

  /**
   * @brief Creates a new entity
   * @return The ID of the created entity
   * @throws std::runtime_error if MAX_ENTITIES is reached
   *
   * Reuses freed entity IDs when available for memory efficiency.
   */
  [[nodiscard]] Entity createEntity()
  {
    if (m_livingEntityCount >= MAX_ENTITIES) {
      throw std::runtime_error("EntityManager: Cannot create more entities (MAX_ENTITIES reached)");
    }

    Entity newEntity;

    // Reuse a free ID if available
    if (!m_freeIds.empty()) {
      newEntity = m_freeIds.back();
      m_freeIds.pop_back();
      m_alive[newEntity] = 1;
    } else {
      // Create a new entity with a new ID
      if (m_nextId >= MAX_ENTITIES) {
        throw std::runtime_error("EntityManager: Entity ID overflow");
      }
      newEntity = m_nextId++;
      m_alive.push_back(1);
    }

    // Reset entity signature
    m_signatures[newEntity].reset();
    ++m_livingEntityCount;

    return newEntity;
  }

  /**
   * @brief Destroys an entity and frees its ID for reuse
   * @param entity The entity to destroy
   *
   * Resets the entity's signature and marks it as available for recycling.
   */
  void destroyEntity(Entity entity)
  {
    if (entity >= m_alive.size() || (m_alive[entity] == 0U)) {
      return; // Entity already destroyed or non-existent
    }

    m_alive[entity] = 0;
    m_signatures[entity].reset();
    m_freeIds.push_back(entity);
    --m_livingEntityCount;
  }

  /**
   * @brief Sets the component signature for an entity
   * @param entity Target entity
   * @param signature New component signature
   * @throws std::out_of_range if entity is not alive
   */
  void setSignature(Entity entity, const ComponentSignature &signature)
  {
    if (entity >= m_alive.size() || !isAlive(entity)) {
      throw std::out_of_range("EntityManager: Entity is not alive");
    }

    m_signatures[entity] = signature;
  }

  /**
   * @brief Retrieves the component signature for an entity
   * @param entity Target entity
   * @return The entity's component signature
   * @throws std::out_of_range if entity is not alive
   */
  [[nodiscard]] const ComponentSignature &getSignature(Entity entity) const
  {
    if (entity >= m_alive.size() || !isAlive(entity)) {
      throw std::out_of_range("EntityManager: Entity is not alive");
    }

    return m_signatures[entity];
  }

  /**
   * @brief Checks if an entity is alive
   * @param entity Entity to check
   * @return true if the entity exists and is alive
   */
  [[nodiscard]] bool isAlive(Entity entity) const { return entity < m_alive.size() && m_alive[entity] != 0; }

  /**
   * @brief Returns the number of living entities
   * @return Number of active entities
   */
  [[nodiscard]] std::size_t getAliveCount() const { return m_livingEntityCount; }

  /**
   * @brief Returns the total number of entities created (alive + dead)
   * @return Total entity count
   */
  [[nodiscard]] std::size_t getTotalCount() const { return m_alive.size(); }

  /**
   * @brief Retrieves all living entity IDs
   * @return Vector of all active entity IDs
   *
   * Useful for iterating over all entities in the world.
   */
  [[nodiscard]] std::vector<Entity> getAllEntities() const
  {
    std::vector<Entity> entities;
    entities.reserve(m_livingEntityCount);

    for (Entity entity = 0; entity < m_alive.size(); ++entity) {
      if (m_alive[entity] != 0) {
        entities.push_back(entity);
      }
    }

    return entities;
  }

  /**
   * @brief Resets the entity manager to initial state
   *
   * Destroys all entities and clears all data structures.
   */
  void clear()
  {
    m_alive.clear();
    m_freeIds.clear();
    m_signatures.clear();
    m_signatures.resize(MAX_ENTITIES);
    m_nextId = 0;
    m_livingEntityCount = 0;
  }

private:
  std::vector<uint8_t> m_alive; ///< Entity states (1 = alive, 0 = dead)
  std::vector<Entity> m_freeIds; ///< Available IDs for reuse
  std::vector<ComponentSignature> m_signatures; ///< Component signatures per entity
  Entity m_nextId; ///< Next ID to assign
  std::size_t m_livingEntityCount; ///< Number of living entities (cached for performance)
};
} // namespace ecs

#endif // ECS_ENTITYMANAGER_HPP_