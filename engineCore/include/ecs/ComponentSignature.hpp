/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ComponentSignature.hpp
*/

#ifndef ECS_COMPONENTSIGNATURE_HPP_
#define ECS_COMPONENTSIGNATURE_HPP_

#include <bitset>
#include <cstddef>

namespace ecs
{
/**
 * @brief Maximum number of component types supported by the ECS
 *
 * This defines the size of the bitset used for component signatures.
 * Each component type gets a unique bit position (0 to MAX_COMPONENTS-1).
 *
 * @note Increasing this value allows more component types but increases memory usage
 * @note 64 components should be sufficient for most game architectures
 */
constexpr std::size_t MAX_COMPONENTS = 64;

/**
 * @brief Type alias for component signatures using bitsets
 *
 * A component signature is a bitset where each bit represents the presence
 * or absence of a specific component type. This allows for extremely fast
 * component queries using bitwise operations.
 *
 * @example
 * ComponentSignature signature;
 * signature.set(0); // Entity has component type 0 (e.g., Position)
 * signature.set(1); // Entity has component type 1 (e.g., Velocity)
 *
 * // Check if entity has both components (single CPU operation)
 * if ((entitySignature & systemSignature) == systemSignature) {
 *     // Entity matches system requirements
 * }
 *
 * @note Bitwise operations on signatures are O(1) and cache-friendly
 * @note This is significantly faster than iterating over component lists
 */
using ComponentSignature = std::bitset<MAX_COMPONENTS>;

/**
 * @brief Component type ID counter for runtime component registration
 *
 * This is used internally to assign unique bit positions to component types.
 * Each component type T gets a unique ID when first registered.
 *
 * @note This is automatically managed by the ComponentManager
 * @internal
 */
inline std::size_t &getNextComponentId() noexcept
{
  static std::size_t nextId = 0;
  return nextId;
}

/**
 * @brief Gets or assigns a unique component ID for type T
 * @tparam T Component type
 * @return Unique bit position for this component type (0 to MAX_COMPONENTS-1)
 *
 * @note This function is thread-safe for reads after first call
 * @note Component IDs are stable for the program lifetime
 *
 * @example
 * auto posId = getComponentId<Position>(); // Returns 0
 * auto velId = getComponentId<Velocity>(); // Returns 1
 * auto posId2 = getComponentId<Position>(); // Returns 0 (same as before)
 */
template <typename T>
std::size_t getComponentId() noexcept
{
  static const std::size_t componentId = getNextComponentId()++;
  return componentId;
}
} // namespace ecs

#endif // ECS_COMPONENTSIGNATURE_HPP_
