/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EventListener.hpp
*/

#ifndef ECS_EVENTS_EVENT_LISTENER_HANDLE_HPP_
#define ECS_EVENTS_EVENT_LISTENER_HANDLE_HPP_

#include <functional>
#include <utility>

namespace ecs
{

class EventListenerHandle
{
public:
  EventListenerHandle() = default;

  EventListenerHandle(std::function<void()> unsubscribeFunc) : unsubscribe(std::move(unsubscribeFunc)) {}

  ~EventListenerHandle()
  {
    if (unsubscribe) {
      unsubscribe();
    }
  }

  EventListenerHandle(const EventListenerHandle &) = delete;
  EventListenerHandle &operator=(const EventListenerHandle &) = delete;

  EventListenerHandle(EventListenerHandle &&other) noexcept : unsubscribe(std::move(other.unsubscribe))
  {
    other.unsubscribe = nullptr;
  }

  EventListenerHandle &operator=(EventListenerHandle &&other) noexcept
  {
    if (this != &other) {
      unsubscribe = std::move(other.unsubscribe);
      other.unsubscribe = nullptr;
    }
    return *this;
  }

private:
  std::function<void()> unsubscribe{};
};

} // namespace ecs

#endif // ECS_EVENTS_EVENT_LISTENER_HANDLE_HPP_
