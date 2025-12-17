/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Score component
*/

#ifndef ECS_SCORE_HPP_
#define ECS_SCORE_HPP_

#include <cstdint>

namespace ecs
{

/**
 * @brief Score component - tracks player score
 */
struct Score {
  int points = 0;
};

} // namespace ecs

#endif // ECS_SCORE_HPP_
