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
   * @brief Check if player is dead and should return to menu
   * @return true if player health is zero or below
   */
  bool shouldReturnToMenu() const { return m_playerHealth <= 0; }

  /**
   * @brief Update player animation based on movement input
   * @param delta_time Time elapsed since last update
   */
  void changeAnimationPlayers(float delta_time);

  /**
   * @brief Reset player animation to idle state
   */
  void resetPlayerAnimation();

private:
  std::shared_ptr<IRenderer> renderer; ///< Renderer interface
  std::shared_ptr<ecs::World> world; ///< ECS world
  std::unique_ptr<ParallaxBackground> background; ///< Scrolling background

  std::unordered_map<std::uint32_t, void *> m_spriteTextures; ///< Sprite texture cache

  // Client-side visual state for brocolis eclosion (visual only — authoritative state remains on server)
  struct BrocolisEclosionState {
    bool active = false;
    float timer = 0.0f; // elapsed time since eclosion started
    float duration = 3.0f; // seconds to hatch (matches server)
    float startScale = 0.1f; // initial small scale
    float targetScale = 1.0f; // expected scale when hatched
    float currentScale = 0.1f; // scale used for rendering
  };
  std::unordered_map<ecs::Entity, BrocolisEclosionState> m_brocolisEclosions;

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

  // FPS tracking
  float m_fpsAccumulator = 0.0f; ///< Time accumulator for FPS calculation
  int m_fpsFrameCount = 0; ///< Frame count for FPS calculation
  float m_currentFps = 0.0f; ///< Current calculated FPS

  std::shared_ptr<INetworkManager> m_networkManager; ///< Network manager for stats
  float m_pingTimer = 0.0f; ///< Timer for sending pings
};
