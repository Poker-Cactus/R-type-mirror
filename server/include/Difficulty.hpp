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
// Easy: 3 hearts = 100 HP
// Medium: 1.5 hearts = 50 HP
// Expert: 0.5 hearts = 17 HP
constexpr int EASY_HP = 100;
constexpr int MEDIUM_HP = 50;
constexpr int EXPERT_HP = 17;

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
