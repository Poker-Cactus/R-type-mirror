/**
 * @file Game.hpp
 * @brief Main game controller and state manager
 */

#pragma once
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/AsioClient.hpp"
#include "../ModuleLoader.hpp"
#include "../include/Settings.hpp"
#include "../include/systems/NetworkReceiveSystem.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/IRenderer.hpp"
#include "LobbyRoomState.hpp"
#include "Menu.hpp"
#include "PlayingState.hpp"
#include <cstdint>
#include <memory>
#include <string>

class INetworkManager;

/**
 * @class Game
 * @brief Main game controller managing lifecycle and state machine
 *
 * Orchestrates the entire game flow including initialization, main loop,
 * state transitions (menu, lobby, gameplay), input handling, and cleanup.
 */
class Game
{
public:
  /**
   * @enum GameState
   * @brief High-level game states
   */
  enum class GameState : std::uint8_t {
    MENU, ///< Main menu
    LOBBY_ROOM, ///< Lobby waiting room
    PLAYING, ///< Active gameplay
    PAUSED ///< Game paused
  };

  /**
   * @brief Construct game with default server connection
   */
  Game();

  /**
   * @brief Construct game with specific server connection
   * @param host Server hostname or IP
   * @param port Server port
   */
  Game(const std::string &host, const std::string &port);
  ~Game();

  /**
   * @brief Initialize game resources and subsystems
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Run the main game loop
   */
  void run();

  /**
   * @brief Shutdown and clean up game resources
   */
  void shutdown();

  /**
   * @brief Set current game state
   * @param newState New game state
   */
  void setState(GameState newState);

  /**
   * @brief Get current game state
   * @return Current game state
   */
  [[nodiscard]] GameState getState() const;

private:
  void processInput();
  void update(float deltaTime);
  void render();
  void ensureInputEntity();
  void sendLeaveToServer();
  void sendViewportToServer();

  void handleMenuStateInput();
  void handleLobbyRoomTransition();
  void handleLobbyRoomStateInput();
  void handlePlayingStateInput();
  void updatePlayerInput();
  void delegateInputToCurrentState();

  std::unique_ptr<Module<IRenderer>> module;
  std::shared_ptr<IRenderer> renderer;
  std::shared_ptr<ecs::World> m_world;
  std::shared_ptr<INetworkManager> m_networkManager;
  bool isRunning = false;
  GameState currentState = GameState::MENU;
  std::string m_serverHost = "127.0.0.1";
  std::string m_serverPort = "4242";
  ecs::Entity m_inputEntity{0};
  std::unique_ptr<Menu> menu;
  std::unique_ptr<LobbyRoomState> lobbyRoomState;
  std::unique_ptr<PlayingState> playingState;
  float m_lobbyStateTime = 0.0F;
  Settings settings;
  bool fullScreen = true;
};
