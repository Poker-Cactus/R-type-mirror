/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LevelProgress.hpp - Tracks player progress through a level
*/

#ifndef ECS_COMPONENTS_LEVEL_PROGRESS_HPP_
#define ECS_COMPONENTS_LEVEL_PROGRESS_HPP_

namespace ecs
{

/**
 * @brief Component that tracks how far a player has progressed through a level
 *
 * This tracks the automatic scrolling distance, not the player's screen position.
 * The world scrolls at a constant speed, and this accumulates the total distance.
 */
struct LevelProgress {
  float distanceTraveled = 0.0f; // Total distance scrolled in the level (in pixels)
};

} // namespace ecs

#endif // ECS_COMPONENTS_LEVEL_PROGRESS_HPP_
