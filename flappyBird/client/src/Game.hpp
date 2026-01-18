/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game.hpp
*/

#pragma once
#include "../../client/ModuleLoader.hpp"
#include "../../client/interface/IRenderer.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/AsioClient.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include "Menu.hpp"

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

private:
  std::unique_ptr<Module<IRenderer>> module;
  std::shared_ptr<IRenderer> renderer;
  std::shared_ptr<ecs::World> m_world;
  std::shared_ptr<INetworkManager> m_networkManager;
  bool isRunning = false;
  GameState currentState = GameState::MENU;
  std::string m_serverHost = "127.0.0.1";
  std::string m_serverPort = "4243";
  std::string m_rendererType = "sfml";
  ecs::Entity m_inputEntity{0};
  float m_lobbyStateTime = 0.0F;
  bool fullScreen = true;

  void sendLeaveToServer();

  // Usefull func
  void render();
  void update(float deltaTime);

  std::unique_ptr<Menu> menu;
  // HighscoreManager highscoreManager;
};
