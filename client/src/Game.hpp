/**
 * @file Game.hpp
 * @brief Main game controller and state manager
 */

#pragma once
#include "../../common/include/Highscore.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/AsioClient.hpp"
#include "../ModuleLoader.hpp"
#include "../include/AudioManager.hpp"
#include "../include/Settings.hpp"
#include "../include/systems/NetworkReceiveSystem.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/IRenderer.hpp"
#include "ChatUI.hpp"
#include "LobbyRoomState.hpp"
#include "Menu.hpp"
#include "PlayingState.hpp"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
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
    PAUSED, ///< Game paused
    VICTORY ///< Victory screen
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

  /**
   * @brief Construct game with specific server connection and renderer
   * @param host Server hostname or IP
   * @param port Server port
   * @param rendererType Renderer type: "sdl2" or "sfml"
   */
  Game(const std::string &host, const std::string &port, const std::string &rendererType);
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
  /** @brief Poll input and update input state. */
  void processInput();
  /** @brief Update game logic for the current state. */
  void update(float deltaTime);
  /** @brief Render the current state and overlays. */
  void render();
  /** @brief Ensure the input entity exists in the ECS world. */
  void ensureInputEntity();
  /** @brief Notify the server that the client is leaving. */
  void sendLeaveToServer();
  /** @brief Send the current viewport size to the server. */
  void sendViewportToServer();
  /** @brief Send a chat message to the server. */
  void sendChatMessage(const std::string &message);
  /** @brief Handle chat input state and message submission. */
  void handleChatInput();

  /** @brief Process input when in the menu state. */
  void handleMenuStateInput();
  /** @brief Transition from lobby menu to lobby room. */
  void handleLobbyRoomTransition();
  /** @brief Process input when in the lobby room state. */
  void handleLobbyRoomStateInput();
  /** @brief Process input when in the playing state. */
  void handlePlayingStateInput();
  /** @brief Process input on the victory screen. */
  void handleVictoryInput();
  /** @brief Update the player input component. */
  void updatePlayerInput();
  /** @brief Delegate input to the currently active state. */
  void delegateInputToCurrentState();
  /** @brief Render the end screen based on payload. */
  void renderEndScreen();
  /** @brief Render the victory screen. */
  void renderVictoryScreen();

  std::unique_ptr<Module<IRenderer>> module;
  std::shared_ptr<IRenderer> renderer;
  std::shared_ptr<ecs::World> m_world;
  std::shared_ptr<INetworkManager> m_networkManager;
  bool isRunning = false;
  GameState currentState = GameState::MENU;
  std::string m_serverHost = "127.0.0.1";
  std::string m_serverPort = "4242";
  std::string m_rendererType = "sfml";
  ecs::Entity m_inputEntity{0};
  std::unique_ptr<Menu> menu;
  std::unique_ptr<LobbyRoomState> lobbyRoomState;
  std::unique_ptr<PlayingState> playingState;
  float m_lobbyStateTime = 0.0F;
  int m_victoryScore = 0; ///< Score at victory
  bool m_showEndScreen = false;
  nlohmann::json m_endScreenPayload;
  bool fullScreen = true;
  ColorBlindMode currentColorBlindMode = ColorBlindMode::NONE;
  HighscoreManager highscoreManager;
  std::unique_ptr<ChatUI> m_chatUI;
  std::shared_ptr<AudioManager> m_audioManager;
  Settings settings;

public:
  /**
   * @brief Get the audio manager
   * @return Shared pointer to audio manager
   */
  std::shared_ptr<AudioManager> getAudioManager() const { return m_audioManager; }
};
