/**
 * @file PlayingState.hpp
 * @brief Active gameplay state management
 */

#pragma once
#include "../../engineCore/include/ecs/World.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../interface/IRenderer.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include "ParallaxBackground.hpp"
#include "infomode/include/InfoMode.hpp"
#include <memory>
#include <unordered_map>

/**
 * @class PlayingState
 * @brief Manages the active gameplay state
 *
 * This class handles all aspects of active gameplay including:
 * - Animated parallax background
 * - Game entities (player, enemies, projectiles)
 * - Player input processing
 * - HUD rendering and updates
 * - Animation system
 */
class PlayingState
{
public:
  /**
   * @brief Construct the playing state
   * @param renderer Renderer interface
   * @param world Shared pointer to ECS world
   * @param settings Game settings reference
   * @param networkManager Network manager for stats
   */
  PlayingState(std::shared_ptr<IRenderer> renderer, const std::shared_ptr<ecs::World> &world, Settings &settings,
               std::shared_ptr<INetworkManager> networkManager);
  ~PlayingState();

  /**
   * @brief Initialize gameplay state
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Update game logic
   * @param delta_time Time elapsed since last update in seconds
   */
  void update(float delta_time);

  /**
   * @brief Render all game elements
   */
  void render();

  /**
   * @brief Process user input during gameplay
   */
  void processInput();

  /**
   * @brief Clean up resources
   */
  void cleanup();

  /**
   * @brief Set solo mode (no network multiplayer)
   * @param isSolo true for solo gameplay
   */
  void setSoloMode(bool isSolo);

  /**
   * @brief Check if this is solo mode
   * @return true if solo mode
   */
  [[nodiscard]] bool isSolo() const { return m_isSolo; }

  /**
   * @brief Check if player is dead and should return to menu
   * @return true if player health is zero or below AND not in spectator mode
   */
  bool shouldReturnToMenu() const { return m_playerHealth <= 0 && !m_isSpectator; }

  /**
   * @brief Get the current player score
   * @return the player score
   */
  [[nodiscard]] int getPlayerScore() const { return m_playerScore; }

  /**
   * @brief Update player animation based on movement input
   * @param delta_time Time elapsed since last update
   */
  void changeAnimationPlayers(float delta_time);

  /**
   * @brief Reset player animation to idle state
   */
  void resetPlayerAnimation();

  /**
   * @brief Enable/disable spectator mode
   * @param enabled true to enable spectator mode
   */
  void setSpectatorMode(bool enabled) { m_isSpectator = enabled; }

  /**
   * @brief Check if in spectator mode
   * @return true if spectating
   */
  bool isSpectator() const { return m_isSpectator; }

private:
  std::shared_ptr<IRenderer> renderer; ///< Renderer interface
  std::shared_ptr<ecs::World> world; ///< ECS world
  std::unique_ptr<ParallaxBackground> background; ///< Scrolling background

  void *m_mapTexture = nullptr; ///< Level map texture
  int m_mapWidth = 0; ///< Map texture width
  int m_mapHeight = 0; ///< Map texture height
  float m_mapOffsetX = 0.0f; ///< Map horizontal scroll offset
  static constexpr float MAP_SCROLL_SPEED = 50.0f; ///< Map scroll speed (pixels/second)

  // Reference resolution for coordinate normalization (same as server)
  static constexpr float REFERENCE_WIDTH = 1920.0F;
  static constexpr float REFERENCE_HEIGHT = 1080.0F;
  
  float m_scaleX = 1.0f; ///< Horizontal scale factor (windowWidth / REFERENCE_WIDTH)
  float m_scaleY = 1.0f; ///< Vertical scale factor (gameAreaHeight / REFERENCE_HEIGHT)
  int m_hudHeight = 0; ///< HUD strip height in pixels (bottom 1/12)
  int m_gameHeight = 0; ///< Game area height in pixels (windowHeight - hudHeight)

  std::unordered_map<std::uint32_t, void *> m_spriteTextures; ///< Sprite texture cache

  /**
   * @brief Load sprite textures into cache
   */
  void loadSpriteTextures();

  /**
   * @brief Free sprite textures from cache
   */
  void freeSpriteTextures();

  // HUD state (discrete lives)
  static constexpr int INITIAL_PLAYER_LIVES = 3;
  std::shared_ptr<void> m_hudFont = nullptr;
  void *m_lifeTexture = nullptr; ///< Texture for life icon
  int m_playerHealth = INITIAL_PLAYER_LIVES; ///< Interpreted as number of lives
  int m_playerMaxHealth = INITIAL_PLAYER_LIVES;
  int m_playerScore = 0;
  /**
   * @brief Update entity animations
   * @param deltaTime Time elapsed since last update
   */
  void updateAnimations(float deltaTime);

  /**
   * @brief Render heads-up display
   */
  void renderHUD();

  /**
   * @brief Update HUD data from ECS world
   */
  void updateHUDFromWorld(float deltaTime);

  bool m_returnUp = false; ///< Return to neutral animation from up
  bool m_returnDown = false; ///< Return to neutral animation from down

  std::unique_ptr<rtype::InfoMode> m_infoMode; ///< Info mode manager

  int m_playerFrameIndex = 2; ///< Current animation frame
  float m_playerAnimTimer = 0.f; ///< Animation timer

  /**
   * @enum PlayerAnimDirection
   * @brief Player animation direction state
   */
  enum class PlayerAnimDirection { None, Up, Down };
  PlayerAnimDirection m_playerAnimDirection = PlayerAnimDirection::None; ///< Current anim direction
  bool m_playerAnimPlayingOnce = false; ///< Single-play animation flag
  int m_playerAnimPhase = 0; ///< Animation phase

  Settings &settings; ///< Game settings reference
  std::shared_ptr<SettingsMenu> settingsMenu; ///< Settings menu
  bool m_isSolo = false; ///< Solo mode flag

  // FPS tracking
  float m_fpsAccumulator = 0.0f; ///< Time accumulator for FPS calculation
  int m_fpsFrameCount = 0; ///< Frame count for FPS calculation
  float m_currentFps = 0.0f; ///< Current calculated FPS

  std::shared_ptr<INetworkManager> m_networkManager; ///< Network manager for stats
  float m_pingTimer = 0.0f; ///< Timer for sending pings

  bool m_isSpectator = false; ///< Spectator mode flag
};
