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
#include <unordered_set>

// Forward declarations
class AudioManager;

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
   * @param audioManager Audio manager for sounds and music
   */
  PlayingState(std::shared_ptr<IRenderer> renderer, const std::shared_ptr<ecs::World> &world, Settings &settings,
               std::shared_ptr<INetworkManager> networkManager, std::shared_ptr<AudioManager> audioManager);
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

  /**
   * @brief Start a level transition with fade effect
   * @param nextLevelId ID of the next level
   */
  void startLevelTransition(const std::string &nextLevelId);

  /**
   * @brief Update level transition state
   * @param deltaTime Time elapsed since last update
   */
  void updateLevelTransition(float deltaTime);

  /**
   * @brief Render the fade overlay
   */
  void renderFadeOverlay();

private:
  std::shared_ptr<IRenderer> renderer; ///< Renderer interface
  std::shared_ptr<ecs::World> world; ///< ECS world
  std::unique_ptr<ParallaxBackground> background; ///< Scrolling background

  std::unordered_map<std::uint32_t, void *> m_spriteTextures; ///< Sprite texture cache

  /**
   * @brief Load sprite textures into cache
   */
  void loadSpriteTextures();

  /**
   * @brief Free sprite textures from cache
   */
  void freeSpriteTextures();

  // HUD state
  static constexpr int INITIAL_PLAYER_HEALTH = 100;
  std::shared_ptr<void> m_hudFont = nullptr;
  void *m_heartsTexture = nullptr;
  int m_playerHealth = INITIAL_PLAYER_HEALTH;
  int m_playerMaxHealth = INITIAL_PLAYER_HEALTH;
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

  // Audio
  std::shared_ptr<AudioManager> m_audioManager; ///< Audio manager for sounds and music

  // Input state tracking for sound effects
  bool m_prevShootPressed = false; ///< Previous shoot key state
  bool m_prevChargedShootPressed = false; ///< Previous charged shoot key state
  float m_chargedShotSoundTimer = -1.0f; ///< Timer to delay charged shot sound by 1 second

  // Entity tracking for sound effects
  std::unordered_set<ecs::Entity> m_previousEnemies; ///< Track enemies from previous frame
  int m_previousEnemyCount = 0; ///< Track enemy count from previous frame

  // Level transition state
  bool m_isTransitioning = false; ///< Is a level transition in progress
  float m_transitionTimer = 0.0f; ///< Timer for transition phases
  float m_fadeAlpha = 0.0f; ///< Current fade alpha (0.0 = transparent, 1.0 = black)
  enum class TransitionPhase {
    NONE,
    FADE_OUT,    // Fade to black
    WAITING,     // Brief pause at black
    FADE_IN      // Fade from black
  };
  TransitionPhase m_transitionPhase = TransitionPhase::NONE;
  std::string m_nextLevelId; ///< ID of the next level to transition to

  /**
   * @brief Load sound effects
   */
  void loadSounds();

  /**
   * @brief Free sound effects
   */
  void freeSounds();

  /**
   * @brief Load level music
   */
  void loadMusic();

  /**
   * @brief Free level music
   */
  void freeMusic();

  /**
   * @brief Update sound effects based on game events
   */
  void updateSounds();
};
