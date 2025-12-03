/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EntityManager.hpp
*/

#ifndef ECS_ENTITYMANAGER_HPP_
#define ECS_ENTITYMANAGER_HPP_

#include "Entity.hpp"
#include "Signature.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

// Nombre maximum d'entités dans le monde
constexpr std::size_t MAX_ENTITIES = 5000;

class EntityManager
{
public:
  EntityManager() : m_nextId(0), m_livingEntityCount(0) { m_signatures.resize(MAX_ENTITIES); }

  /**
   * @brief Crée une nouvelle entité
   * @return L'ID de l'entité créée
   * @throws std::runtime_error si MAX_ENTITIES est atteint
   */
  [[nodiscard]] Entity createEntity()
  {
    if (m_livingEntityCount >= MAX_ENTITIES) {
      throw std::runtime_error("EntityManager: Cannot create more entities (MAX_ENTITIES reached)");
    }

    Entity newEntity;

    // Réutiliser un ID libre si disponible
    if (!m_freeIds.empty()) {
      newEntity = m_freeIds.back();
      m_freeIds.pop_back();
      m_alive[newEntity] = 1;
    } else {
      // Créer une nouvelle entité avec un nouvel ID
      if (m_nextId >= MAX_ENTITIES) {
        throw std::runtime_error("EntityManager: Entity ID overflow");
      }
      newEntity = m_nextId++;
      m_alive.push_back(1);
    }

    // Réinitialiser la signature de l'entité
    m_signatures[newEntity].reset();
    ++m_livingEntityCount;

    return newEntity;
  }

  /**
   * @brief Détruit une entité et libère son ID pour réutilisation
   * @param entity L'entité à détruire
   */
  void destroyEntity(Entity entity)
  {
    if (entity >= m_alive.size() || (m_alive[entity] == 0U)) {
      return; // Entité déjà détruite ou inexistante
    }

    m_alive[entity] = 0;
    m_signatures[entity].reset(); // Réinitialiser la signature
    m_freeIds.push_back(entity);
    --m_livingEntityCount;
  }

  /**
   * @brief Définit la signature d'une entité
   * @param entity L'entité cible
   * @param signature La nouvelle signature
   */
  void setSignature(Entity entity, const Signature &signature)
  {
    if (entity >= m_alive.size() || !isAlive(entity)) {
      throw std::out_of_range("EntityManager: Entity is not alive");
    }

    m_signatures[entity] = signature;
  }

  /**
   * @brief Récupère la signature d'une entité
   * @param entity L'entité cible
   * @return La signature de l'entité
   */
  [[nodiscard]] const Signature &getSignature(Entity entity) const
  {
    if (entity >= m_alive.size() || !isAlive(entity)) {
      throw std::out_of_range("EntityManager: Entity is not alive");
    }

    return m_signatures[entity];
  }

  /**
   * @brief Vérifie si une entité est vivante
   * @param entity L'entité à vérifier
   * @return true si l'entité existe et est vivante
   */
  [[nodiscard]] bool isAlive(Entity entity) const { return entity < m_alive.size() && m_alive[entity] != 0; }

  /**
   * @brief Retourne le nombre total d'entités vivantes
   * @return Nombre d'entités actives
   */
  [[nodiscard]] std::size_t getAliveCount() const { return m_livingEntityCount; }

  /**
   * @brief Retourne le nombre total d'entités créées (vivantes + mortes)
   * @return Nombre total d'entités
   */
  [[nodiscard]] std::size_t getTotalCount() const { return m_alive.size(); }

  /**
   * @brief Réinitialise le gestionnaire d'entités
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
  std::vector<uint8_t> m_alive; // États des entités (1 = vivante, 0 = morte)
  std::vector<Entity> m_freeIds; // IDs disponibles pour réutilisation
  std::vector<Signature> m_signatures; // Signatures des entités
  Entity m_nextId; // Prochain ID à attribuer
  std::size_t m_livingEntityCount; // Nombre d'entités vivantes (cache pour perf)
};

#endif // ECS_ENTITYMANAGER_HPP_