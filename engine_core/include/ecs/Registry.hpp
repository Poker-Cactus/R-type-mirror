/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry header
*/

#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <cstddef>

namespace ECS
{

/**
 * @brief Main ECS Registry
 *
 * Manages entities, components and systems
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
     * @return Entity ID
     */
    std::size_t createEntity();

  private:
    std::size_t _nextEntityId{0};
};

} // namespace ECS

#endif // REGISTRY_HPP