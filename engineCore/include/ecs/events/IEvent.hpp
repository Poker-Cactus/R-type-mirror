/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IEvent.hpp
*/

#ifndef ECS_IEVENT_HPP_
#define ECS_IEVENT_HPP_

namespace ecs
{
/**
 * @brief Base interface for all ECS events
 *
 * Events are simple data-only messages passed between systems through the EventBus.
 * This interface exists so event types can be erased or stored in type-index maps.
 */
struct IEvent {
  virtual ~IEvent() = default;
};

} // namespace ecs

#endif // ECS_IEVENT_HPP_
