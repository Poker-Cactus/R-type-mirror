/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InfoMode.cpp
*/

#include "InfoMode.hpp"
#include "categories/CpuRamCategory.hpp"
#include "categories/GameCategory.hpp"
#include "categories/DeviceCategory.hpp"
#include "categories/NetworkCategory.hpp"
#include <iostream>
#include <iomanip>

InfoMode::InfoMode(std::shared_ptr<IRenderer> renderer, void *hudFont)
    : m_renderer(std::move(renderer)), m_hudFont(hudFont)
{
  initializeCategories();
  setupCategoryLayout();
  m_isActive = false;
}

void InfoMode::initializeCategories()
{
  // Create all info categories
  m_categories.emplace_back(std::make_unique<CpuRamCategory>());
  m_categories.emplace_back(std::make_unique<GameCategory>());
  m_categories.emplace_back(std::make_unique<DeviceCategory>());
  m_categories.emplace_back(std::make_unique<NetworkCategory>());
}

void InfoMode::setupCategoryLayout()
{
  // Configure 2x2 layout: 2 categories on left, 2 on right
  const int LEFT_COLUMN_X = 0;
  const int RIGHT_COLUMN_X = 250;
  const int TOP_ROW_Y = 0;
  const int BOTTOM_ROW_Y = 200;  // Increased from 120 to accommodate more lines in categories

  // Left column - top and bottom
  m_categoryPositions.push_back({0, LEFT_COLUMN_X, TOP_ROW_Y});    // CPU & RAM (top-left)
  m_categoryPositions.push_back({1, LEFT_COLUMN_X, BOTTOM_ROW_Y}); // Game (bottom-left)

  // Right column - top and bottom
  m_categoryPositions.push_back({2, RIGHT_COLUMN_X, TOP_ROW_Y});    // Device (top-right)
  m_categoryPositions.push_back({3, RIGHT_COLUMN_X, BOTTOM_ROW_Y}); // Network (bottom-right)
}

void InfoMode::processInput()
{
  if (m_renderer == nullptr)
    return;

  // Toggle info mode with "i" key - only on key press, not hold
  bool iKeyPressed = m_renderer->isKeyPressed(KeyCode::KEY_I);
  
  if (iKeyPressed && !m_iKeyWasPressed) {
    // Key was just pressed (transition from not pressed to pressed)
    m_isActive = !m_isActive;
    std::cout << "[InfoMode] Info mode toggled: " << (m_isActive ? "ON" : "OFF") << '\n';
  }
  
  m_iKeyWasPressed = iKeyPressed;
}

void InfoMode::render(int x, int y)
{
  if (!m_isActive || m_renderer == nullptr || m_hudFont == nullptr || m_categories.empty())
    return;

  const int LINE_HEIGHT = 20;
  const int TITLE_OFFSET = 25;

  // Render all categories at their configured positions
  for (const auto& position : m_categoryPositions) {
    if (position.categoryIndex >= m_categories.size())
      continue;

    InfoCategory* category = m_categories[position.categoryIndex].get();
    if (!category)
      continue;

    // Calculate final position
    int categoryX = x + position.offsetX;
    int categoryY = y + position.offsetY;

    // Render category title
    std::string title = "[" + category->getName() + "]";
    m_renderer->drawText(m_hudFont, title, categoryX, categoryY, HUD_TEXT_WHITE);

    // Render category information lines
    const auto& infoLines = category->getInfoLines();
    int currentY = categoryY + TITLE_OFFSET;

    for (const auto& line : infoLines) {
      m_renderer->drawText(m_hudFont, line, categoryX, currentY, HUD_TEXT_WHITE);
      currentY += LINE_HEIGHT;
    }
  }
}

void InfoMode::update(float deltaTime)
{
  // Update all categories
  for (auto& category : m_categories) {
    category->update(deltaTime);
  }
}

void InfoMode::toggle()
{
  m_isActive = !m_isActive;
  std::cout << "[InfoMode] Info mode toggled: " << (m_isActive ? "ON" : "OFF") << '\n';
}

void InfoMode::setGameData(int health, int score, float fps)
{
  // Find game category and update its data
  for (auto& category : m_categories) {
    if (auto* gameCategory = dynamic_cast<GameCategory*>(category.get())) {
      gameCategory->setPlayerHealth(health);
      gameCategory->setPlayerScore(score);
      gameCategory->setGameFps(fps);
      break;
    }
  }
}

void InfoMode::setGameStats(int entityCount, int playerCount, int enemyCount, int projectileCount, float gameTime)
{
  // Find game category and update its statistics
  for (auto& category : m_categories) {
    if (auto* gameCategory = dynamic_cast<GameCategory*>(category.get())) {
      gameCategory->setEntityCount(entityCount);
      gameCategory->setPlayerCount(playerCount);
      gameCategory->setEnemyCount(enemyCount);
      gameCategory->setProjectileCount(projectileCount);
      gameCategory->setGameTime(gameTime);
      break;
    }
  }
}

void InfoMode::setNetworkData(float latency, bool connected, int packetsPerSecond)
{
  // Find network category and update its data
  for (auto& category : m_categories) {
    if (auto* networkCategory = dynamic_cast<NetworkCategory*>(category.get())) {
      networkCategory->setLatency(latency);
      networkCategory->setConnected(connected);
      networkCategory->setPacketsPerSecond(packetsPerSecond);
      break;
    }
  }
}

void InfoMode::setNetworkBandwidth(int uploadBytes, int downloadBytes)
{
  // Find network category and update its bandwidth data
  for (auto& category : m_categories) {
    if (auto* networkCategory = dynamic_cast<NetworkCategory*>(category.get())) {
      networkCategory->setBandwidth(uploadBytes, downloadBytes);
      break;
    }
  }
}

// yep
