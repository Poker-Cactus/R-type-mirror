/**
 * @file InfoMode.hpp
 * @brief Info mode management for displaying categorized system stats
 */

#pragma once

#include "../interface/IRenderer.hpp"
#include "../interface/KeyCodes.hpp"
#include "InfoCategory.hpp"
#include <memory>
#include <vector>
#include <string>

/**
 * @struct CategoryPosition
 * @brief Defines the position and layout for a category
 */
struct CategoryPosition {
  size_t categoryIndex;  ///< Index of the category in m_categories
  int offsetX;          ///< X offset from base position
  int offsetY;          ///< Y offset from base position
};

/**
 * @class InfoMode
 * @brief Manages the info mode toggle and display with multiple categories
 *
 * This class handles toggling info mode on/off with the 'I' key
 * and rendering all categories in a configurable layout.
 */
class InfoMode
{
public:
  /**
   * @brief Construct the info mode
   * @param renderer Renderer interface
   * @param hudFont Font for HUD text rendering
   */
  InfoMode(std::shared_ptr<IRenderer> renderer, void *hudFont);

  /**
   * @brief Destructor
   */
  ~InfoMode() = default;

  /**
   * @brief Process input for info mode toggle
   */
  void processInput();

  /**
   * @brief Render info mode text if active
   * @param x X position for text
   * @param y Y position for text
   */
  void render(int x, int y);

  /**
   * @brief Update all info categories
   * @param deltaTime Time elapsed since last update in seconds
   */
  void update(float deltaTime);

  /**
   * @brief Check if info mode is currently active
   * @return true if info mode is on
   */
  bool isActive() const { return m_isActive; }

  /**
   * @brief Manually toggle info mode
   */
  void toggle();

  /**
   * @brief Set game-related data for the game category
   * @param health Player health
   * @param score Player score
   * @param fps Game FPS
   */
  void setGameData(int health, int score, float fps);

  /**
   * @brief Set detailed game statistics
   * @param entityCount Total entity count
   * @param playerCount Player entity count
   * @param enemyCount Enemy entity count
   * @param projectileCount Projectile entity count
   * @param gameTime Game time in seconds
   */
  void setGameStats(int entityCount, int playerCount, int enemyCount, int projectileCount, float gameTime);

  /**
   * @brief Set network data for the network category
   * @param latency Network latency in ms
   * @param connected Connection status
   * @param packetsPerSecond Packets per second
   */
  void setNetworkData(float latency, bool connected, int packetsPerSecond);

private:
  /**
   * @brief Initialize all info categories
   */
  void initializeCategories();

  /**
   * @brief Setup the layout positions for categories
   */
  void setupCategoryLayout();

  std::shared_ptr<IRenderer> m_renderer; ///< Renderer interface
  void *m_hudFont; ///< HUD font for text rendering
  bool m_isActive = false; ///< Info mode state

  std::vector<std::unique_ptr<InfoCategory>> m_categories; ///< All available categories
  std::vector<CategoryPosition> m_categoryPositions; ///< Positions for each category

  static constexpr Color HUD_TEXT_WHITE = {.r = 255, .g = 255, .b = 255, .a = 255}; ///< White color for text
};
