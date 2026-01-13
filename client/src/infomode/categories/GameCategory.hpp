/**
 * @file GameCategory.hpp
 * @brief Game information category
 */

#pragma once

#include "../InfoCategory.hpp"

/**
 * @class GameCategory
 * @brief Displays game-related information
 */
class GameCategory : public InfoCategory
{
public:
  /**
   * @brief Constructor
   */
  GameCategory();

  /**
   * @brief Destructor
   */
  ~GameCategory() override = default;

  /**
   * @brief Get category name
   * @return "Game"
   */
  std::string getName() const override { return "Game"; }

  /**
   * @brief Get information lines
   * @return Vector with game info (placeholder for now)
   */
  std::vector<std::string> getInfoLines() const override;

  /**
   * @brief Update game information
   */
  void update() override;

  /**
   * @brief Get the update interval for this category in frames (at 60 FPS)
   * @return 30 frames (every 0.5 seconds at 60 FPS)
   */
  int getUpdateIntervalFrames() const override { return 30; }

  /**
   * @brief Set player health for display
   * @param health Current player health
   */
  void setPlayerHealth(int health) { m_playerHealth = health; }

  /**
   * @brief Set player score for display
   * @param score Current player score
   */
  void setPlayerScore(int score) { m_playerScore = score; }

  /**
   * @brief Set game FPS for display
   * @param fps Current frames per second
   */
  void setGameFps(float fps) { m_gameFps = fps; }

  /**
   * @brief Set total entity count
   * @param count Total number of entities in the game world
   */
  void setEntityCount(int count) { m_entityCount = count; }

  /**
   * @brief Set player entity count
   * @param count Number of player entities
   */
  void setPlayerCount(int count) { m_playerCount = count; }

  /**
   * @brief Set enemy entity count
   * @param count Number of enemy entities
   */
  void setEnemyCount(int count) { m_enemyCount = count; }

  /**
   * @brief Set projectile count
   * @param count Number of projectile entities
   */
  void setProjectileCount(int count) { m_projectileCount = count; }

  /**
   * @brief Set game time
   * @param time Game time in seconds
   */
  void setGameTime(float time) { m_gameTime = time; }

private:
  int m_playerHealth = 0;
  int m_playerScore = 0;
  float m_gameFps = 0.0f;
  int m_entityCount = 0;
  int m_playerCount = 0;
  int m_enemyCount = 0;
  int m_projectileCount = 0;
  float m_gameTime = 0.0f;
};
