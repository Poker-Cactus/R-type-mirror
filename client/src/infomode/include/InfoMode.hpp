/**
 * @file InfoMode.hpp
 * @brief R-Type style debug overlay with configurable real-time monitoring
 *
 * Displays system metrics (CPU/RAM), game stats (FPS, entities), and network info
 * in a horizontal two-column layout. All metrics are configurable via Settings.
 */

#pragma once

#include "../../../interface/IRenderer.hpp"
#include "../../../../engineCore/include/ecs/World.hpp"
#include "../../../../engineCore/include/ecs/EngineComponents.hpp"
#include <chrono>
#include <memory>
#include <string>
#include "../../../../engineCore/include/ecs/components/MapCollision.hpp"

class Settings;

namespace rtype
{

/**
 * @class InfoMode
 * @brief Arcade-style debug overlay with real-time CPU/RAM monitoring
 */
class InfoMode
{
public:
  /**
   * @brief Construct InfoMode with renderer, font, and settings reference
   * @param renderer Shared renderer for drawing operations
   * @param hudFont Shared font resource (managed externally)
   * @param settings Reference to game settings (for toggle states)
   */
  InfoMode(std::shared_ptr<IRenderer> renderer, std::shared_ptr<void> hudFont, Settings &settings);
  ~InfoMode() = default;

  /** @brief Process keyboard input for toggling display (I key) */
  void processInput();

  /** @brief Render debug overlay if active */
  void render();

  /** @brief Update system metrics (conditional based on settings) */
  void update();

  /** @brief Check if overlay is currently visible */
  bool isActive() const { return m_isActive; }

  /** @brief Set overlay visibility state */
  void setActive(bool active) { m_isActive = active; }

  /** @brief Update game data (health, score, FPS) */
  void setGameData(int health, int score, float fps);

  /** @brief Update entity statistics */
  void setGameStats(int entityCount, int playerCount, int enemyCount, int projectileCount, float gameTime);

  /** @brief Update network metrics */
  void setNetworkData(float latency, bool connected, int packetsPerSecond);

  /** @brief Update bandwidth statistics */
  void setNetworkBandwidth(int uploadBytes, int downloadBytes);

  /** @brief Render hitboxes for all entities with colliders
   *  @param world World pointer
   *  @param entityScaleX Scale applied to entity coordinates (usually reference scale)
   *  @param entityScaleY Scale applied to entity coordinates
   */
  void renderHitboxes(const std::shared_ptr<ecs::World> &world, float entityScaleX = 1.0f,
                       float entityScaleY = 1.0f);

private:
  /** @brief Fetch static system info once (OS, CPU name, cores, total RAM) */
  void initStaticSystemInfo();

  /** @brief Update dynamic metrics (CPU/RAM usage) using platform APIs */
  void updateDynamicSystemInfo();

  /** @brief Calculate panel dimensions and positioning from screen size */
  void calculateLayout();

  /** @brief Render main panel with two-column layout */
  void drawPanel();

  /** @brief Draw section header at X position (modifies y) */
  void drawSection(const std::string &title, int x, int &y);

  /** @brief Draw label-value pair at X position (modifies y) */
  void drawLine(const std::string &label, const std::string &value, int x, int &y, bool highlight = false);

  /** @brief Draw progress bar (unused, kept for compatibility) */
  void drawProgressBar(const std::string &label, float percent, int &y);

  /** @brief Add vertical spacing (unused, kept for compatibility) */
  void drawSeparator(int &y);

  // Core dependencies
  std::shared_ptr<IRenderer> m_renderer; ///< Renderer for drawing operations
  std::shared_ptr<void> m_hudFont; ///< HUD font resource
  Settings &m_settings; ///< Game settings reference

  // UI state
  bool m_isActive = false; ///< Overlay visibility
  bool m_keyWasPressed = false; ///< Previous frame key state

  // Static system info (fetched once at startup)
  std::string m_osName; ///< Operating system name
  std::string m_cpuName; ///< CPU model (truncated to 30 chars)
  int m_cpuCores = 0; ///< Number of CPU cores
  uint64_t m_totalRamBytes = 0; ///< Total system RAM in bytes

  // Dynamic system info (updated every 500ms)
  float m_cpuUsage = 0.0f; ///< CPU usage percentage (0-100)
  uint64_t m_usedRamBytes = 0; ///< Used RAM in bytes
  std::chrono::steady_clock::time_point m_lastSystemUpdate; ///< Last update timestamp

#if defined(__APPLE__) || defined(__linux__)
  uint64_t m_prevIdleTime = 0; ///< Previous idle CPU time (for delta calculation)
  uint64_t m_prevTotalTime = 0; ///< Previous total CPU time (for delta calculation)
#endif

  // Game metrics
  int m_health = 0; ///< Player health
  int m_score = 0; ///< Player score
  float m_fps = 0.0f; ///< Frames per second

  // Entity statistics
  int m_entityCount = 0; ///< Total entities in game
  int m_playerCount = 0; ///< Number of players
  int m_enemyCount = 0; ///< Number of enemies (unused)
  int m_projectileCount = 0; ///< Number of projectiles (unused)
  float m_gameTime = 0.0f; ///< Elapsed game time in seconds

  // Network statistics
  float m_latency = 0.0f; ///< Network latency in milliseconds
  bool m_connected = false; ///< Connection status
  int m_packetsPerSecond = 0; ///< Network packets/sec (unused)
  int m_uploadBytes = 0; ///< Upload bandwidth (unused)
  int m_downloadBytes = 0; ///< Download bandwidth (unused)

  // Dynamic layout (calculated from screen size in calculateLayout())
  int m_panelX = 0; ///< Panel top-left X coordinate
  int m_panelY = 0; ///< Panel top-left Y coordinate
  int m_panelWidth = 0; ///< Panel width (35% of screen, 450-650px)
  int m_panelHeight = 0; ///< Panel height (auto-calculated)
  int m_lineHeight = 0; ///< Line spacing (2.5% of screen height)
  int m_sectionGap = 0; ///< Gap between sections
  int m_padding = 0; ///< Internal padding
  int m_labelWidth = 0; ///< Width of label column (fixed 90px)
  int m_barHeight = 0; ///< Progress bar height

  // Map collision debug overlay (disabled)
  // Layout proportions (percentage of screen)
  static constexpr float PANEL_MARGIN_RATIO = 0.01f; // 1% margin from edge
  static constexpr float LINE_HEIGHT_RATIO = 0.025f; // 2.5% of screen height
  static constexpr float PADDING_RATIO = 0.012f; // 1.2% padding
  static constexpr int SYSTEM_UPDATE_MS = 500;
};

} // namespace rtype
