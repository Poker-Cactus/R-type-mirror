/*
** EPITECH PROJECT, 2025
** rtype (Workspace)
** File description:
** SystemManager.hpp
*/

#ifndef ECS_SYSTEMMANAGER_HPP_
#define ECS_SYSTEMMANAGER_HPP_

#include "ISystem.hpp"

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecs
{
/**
 * @brief Manages the lifecycle and execution of systems in the ECS architecture
 *
 * The SystemManager provides type-safe registration, retrieval, and removal of systems.
 * Systems are stored by their type and updated in the order they were registered.
 *
 * @note Systems are stored using std::unique_ptr for automatic memory management
 * @note Registering the same system type twice returns the existing instance
 * @note Systems are executed in registration order (guaranteed by std::vector)
 */
class SystemManager
{
public:
  SystemManager() = default;
  ~SystemManager() = default;

  /**
   * @brief Registers a system in the manager
   * @tparam T System type (must inherit from ISystem)
   * @tparam Args Constructor argument types
   * @param args Arguments forwarded to T's constructor
   * @return Reference to the registered system
   * @note If system already exists, returns existing instance without creating a new one
   */
  template <typename T, typename... Args>
  T &registerSystem(Args &&...args)
  {
    static_assert(std::is_base_of_v<ISystem, T>, "T must derive from ecs::ISystem");

    auto key = std::type_index(typeid(T));
    auto iter = systemLookup.find(key);
    if (iter != systemLookup.end()) {
      return static_cast<T &>(*systems[iter->second]);
    }

    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    T &ref = *ptr;

    std::size_t index = systems.size();
    systems.push_back(std::move(ptr));
    systemLookup.emplace(key, index);

    return ref;
  } /**
     * @brief Retrieves a non-const pointer to a registered system
     * @tparam T System type to retrieve
     * @return Pointer to the system, or nullptr if not found
     */
  template <typename T>
  T *getSystem()
  {
    auto key = std::type_index(typeid(T));
    auto iter = systemLookup.find(key);
    if (iter == systemLookup.end()) {
      return nullptr;
    }
    return static_cast<T *>(systems[iter->second].get());
  }

  /**
   * @brief Retrieves a const pointer to a registered system
   * @tparam T System type to retrieve
   * @return Const pointer to the system, or nullptr if not found
   */
  template <typename T>
  [[nodiscard]] const T *getSystem() const
  {
    auto key = std::type_index(typeid(T));
    auto iter = systemLookup.find(key);
    if (iter == systemLookup.end()) {
      return nullptr;
    }
    return static_cast<const T *>(systems[iter->second].get());
  }

  /**
   * @brief Checks if a system of type T is registered
   * @tparam T System type to check
   * @return true if system is registered, false otherwise
   */
  template <typename T>
  [[nodiscard]] bool hasSystem() const noexcept
  {
    auto key = std::type_index(typeid(T));
    return systemLookup.contains(key);
  }

  /**
   * @brief Removes a system from the manager
   * @tparam T System type to remove
   * @note Does nothing if the system is not registered
   */
  template <typename T>
  void removeSystem() noexcept
  {
    auto key = std::type_index(typeid(T));
    auto iter = systemLookup.find(key);
    if (iter != systemLookup.end()) {
      std::size_t index = iter->second;

      // Remove from vector (swap with last element for efficiency)
      if (index < systems.size() - 1) {
        // Get the last system and update its lookup entry
        ISystem *lastSystem = systems.back().get();
        auto lastKey = std::type_index(typeid(*lastSystem));
        systemLookup[lastKey] = index;
        systems[index] = std::move(systems.back());
      }
      systems.pop_back();
      systemLookup.erase(iter);
    }
  }

  /**
   * @brief Updates all registered systems
   * @param world Reference to the world containing entities and components
   * @param deltaTime Time elapsed since last update (in seconds)
   * @note Systems are updated in the order they were registered (guaranteed)
   */
  void update(World &world, float deltaTime)
  {
    for (auto &system : systems) {
      system->update(world, deltaTime);
    }
  }

  /**
   * @brief Returns the number of registered systems
   * @return Number of systems currently managed
   */
  [[nodiscard]] std::size_t getSystemCount() const noexcept { return systems.size(); }

  /**
   * @brief Removes all registered systems
   * @note All system pointers become invalid after this call
   */
  void clear() noexcept
  {
    systems.clear();
    systemLookup.clear();
  }

private:
  std::vector<std::unique_ptr<ISystem>> systems;
  std::unordered_map<std::type_index, std::size_t> systemLookup;
};
} // namespace ecs

#endif // ECS_SYSTEMMANAGER_HPP_