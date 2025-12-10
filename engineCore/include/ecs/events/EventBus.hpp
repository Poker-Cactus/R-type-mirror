/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EventBus.hpp
*/

#ifndef ECS_EVENTBUS_HPP_
#define ECS_EVENTBUS_HPP_

#include "EventListenerHandle.hpp"
#include "IEvent.hpp"

#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace ecs
{

/**
 * @brief Lightweight, type-safe event messaging bus between ECS systems
 *
 * Features:
 * - subscribe<T>() adds a callback for event type T
 * - emit<T>() dispatches to all listeners of type T
 * - automatic unsubscribe via EventListenerHandle (RAII)
 *
 * Events DO NOT store state or accumulate over frames unless you implement a queue.
 */
class EventBus
{
public:
  EventBus() = default;
  ~EventBus() = default;

  /**
   * @brief Subscribe to an event type T
   * @tparam T event type (must derive from IEvent)
   * @param callback function taking const T&
   * @return an RAII handle that automatically unsubscribes
   */
  template <typename T>
  EventListenerHandle subscribe(std::function<void(const T &)> callback)
  {
    static_assert(std::is_base_of_v<IEvent, T>, "Event type must derive from IEvent");

    std::type_index key(typeid(T));
    auto &list = listeners[key];

    list.push_back(
      [callback_fn = std::move(callback)](const IEvent &evt) { callback_fn(static_cast<const T &>(evt)); });

    std::size_t index = list.size() - 1;

    // RAII handle will remove this listener when destroyed
    return EventListenerHandle([this, key, index]() {
      auto &vec = listeners[key];
      if (index < vec.size()) {
        vec[index] = nullptr; // mark as removed
      }
    });
  }

  /**
   * @brief Emit/broadcast an event to all listeners of type T
   * @tparam T the event type
   * @param evt event instance
   */
  template <typename T>
  void emit(const T &evt)
  {
    static_assert(std::is_base_of_v<IEvent, T>, "Event type must derive from IEvent");

    std::type_index key(typeid(T));
    auto iter = listeners.find(key);
    if (iter == listeners.end()) {
      return;
    }

    for (auto &callback : iter->second) {
      if (callback) {
        callback(evt);
      }
    }
  }

  /** @brief Remove all listeners for all event types */
  void clear() { listeners.clear(); }

private:
  // key = event type, value = list of callbacks taking const IEvent&
  std::unordered_map<std::type_index, std::vector<std::function<void(const IEvent &)>>> listeners;
};

} // namespace ecs

#endif // ECS_EVENTBUS_HPP_
