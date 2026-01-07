/**
 * @file PlayingState.hpp
 * @brief Active gameplay state management
 */

#pragma once
#include "../../engineCore/include/ecs/World.hpp"
#include "../interface/IRenderer.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include "ParallaxBackground.hpp"
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
   */
  PlayingState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world, Settings &settings);
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

private:
  IRenderer *renderer;                   ///< Renderer interface
  std::shared_ptr<ecs::World> world;     ///< ECS world
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
  void *m_hudFont = nullptr;
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
  void updateHUDFromWorld();

  bool m_returnUp = false;   ///< Return to neutral animation from up
  bool m_returnDown = false; ///< Return to neutral animation from down

  int m_playerFrameIndex = 2;    ///< Current animation frame
  int m_playerAnimToggle = 0;    ///< Animation toggle state
  float m_playerAnimTimer = 0.f; ///< Animation timer

  /**
   * @enum PlayerAnimDirection
   * @brief Player animation direction state
   */
  enum class PlayerAnimDirection { None, Up, Down };
  PlayerAnimDirection m_playerAnimDirection = PlayerAnimDirection::None; ///< Current anim direction
  bool m_playerAnimPlayingOnce = false; ///< Single-play animation flag
  int m_playerAnimPhase = 0;            ///< Animation phase

  Settings &settings;                   ///< Game settings reference
  SettingsMenu *settingsMenu = nullptr; ///< Settings menu
};
