/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry.hpp - Main ECS Registry
*/

#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <cstddef>

namespace rtype::ecs
{

/**
 * @brief Main ECS Registry
 *
 * Manages entities, components and systems.
 */
class Registry
{
  public:
    Registry() = default;
    ~Registry() = default;

    // Delete copy semantics
    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;

    // Allow move semantics
    Registry(Registry &&) noexcept = default;
    Registry &operator=(Registry &&) noexcept = default;

    /**
     * @brief Create a new entity
     *
     * @return Entity ID
     */
    std::size_t createEntity();

  private:
    std::size_t m_nextEntityId{0};
};

} // namespace rtype::ecs

#endif // REGISTRY_HPP_