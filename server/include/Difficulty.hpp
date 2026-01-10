/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Difficulty configuration
*/

#ifndef DIFFICULTY_HPP_
#define DIFFICULTY_HPP_

namespace GameConfig
{
// Difficulty system
enum class Difficulty { EASY, MEDIUM, EXPERT };

// Difficulty-based HP values
// Easy: More HP for easier survival (150 HP)
// Medium: Standard HP (100 HP)
// Expert: Less HP for challenge (75 HP)
constexpr int EASY_HP = 300;
constexpr int MEDIUM_HP = 150;
constexpr int EXPERT_HP = 50;

/**
 * @brief Get player starting HP based on difficulty
 * @param difficulty The game difficulty
 * @return Starting HP value
 */
inline int getPlayerHPForDifficulty(Difficulty difficulty)
{
  switch (difficulty) {
  case Difficulty::EASY:
    return EASY_HP;
  case Difficulty::MEDIUM:
    return MEDIUM_HP;
  case Difficulty::EXPERT:
    return EXPERT_HP;
  default:
    return MEDIUM_HP;
  }
}
} // namespace GameConfig

#endif /* !DIFFICULTY_HPP_ */
