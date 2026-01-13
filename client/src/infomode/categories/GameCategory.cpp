/**
 * @file GameCategory.cpp
 * @brief Implementation of game information category
 */

#include "GameCategory.hpp"
#include <sstream>
#include <iomanip>

GameCategory::GameCategory()
{
}

std::vector<std::string> GameCategory::getInfoLines() const
{
  std::vector<std::string> lines;

  // Player Health
  std::stringstream healthText;
  healthText << "Health: " << m_playerHealth;
  lines.push_back(healthText.str());

  // Player Score
  std::stringstream scoreText;
  scoreText << "Score: " << m_playerScore;
  lines.push_back(scoreText.str());

  // Game FPS
  std::stringstream fpsText;
  if (m_gameFps > 0.0f) {
    fpsText << std::fixed << std::setprecision(1) << "FPS: " << m_gameFps;
  } else {
    fpsText << "FPS: --";
  }
  lines.push_back(fpsText.str());

  // Entity counts
  std::stringstream entityText;
  entityText << "Entities: " << m_entityCount << " total";
  lines.push_back(entityText.str());

  // Breakdown by type
  if (m_playerCount > 0) {
    std::stringstream playerText;
    playerText << "Players: " << m_playerCount;
    lines.push_back(playerText.str());
  }

  if (m_enemyCount > 0) {
    std::stringstream enemyText;
    enemyText << "Enemies: " << m_enemyCount;
    lines.push_back(enemyText.str());
  }

  if (m_projectileCount > 0) {
    std::stringstream projectileText;
    projectileText << "Projectiles: " << m_projectileCount;
    lines.push_back(projectileText.str());
  }

  // Game time
  if (m_gameTime > 0.0f) {
    int minutes = static_cast<int>(m_gameTime) / 60;
    int seconds = static_cast<int>(m_gameTime) % 60;
    std::stringstream timeText;
    timeText << "Time: " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    lines.push_back(timeText.str());
  }

  return lines;
}

void GameCategory::update(float deltaTime)
{
  // Game category doesn't need frequent updates
  // Player stats are updated externally via setters
}
