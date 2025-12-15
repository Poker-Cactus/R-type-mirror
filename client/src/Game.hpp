#pragma once
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/AsioClient.hpp"
#include "../ModuleLoader.hpp"
#include "../include/systems/NetworkReceiveSystem.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/IRenderer.hpp"
#include "LobbyRoomState.hpp"
#include "Menu.hpp"
#include "PlayingState.hpp"
#include <cstdint>
#include <memory>

class INetworkManager;

/**
 * @brief Main game controller managing lifecycle, states, and game loop
 */
class Game
{
public:
  enum class GameState : std::uint8_t { MENU, LOBBY_ROOM, PLAYING, PAUSED };

  Game();
  ~Game();

  bool init();
  void run();
  void shutdown();

  void setState(GameState newState);
  [[nodiscard]] GameState getState() const;

private:
  void processInput();
  void update(float deltaTime);
  void render();
  void ensureInputEntity();

  void handleMenuStateInput();
  void handleLobbyRoomTransition();
  void handlePlayingStateInput();
  void updatePlayerInput();
  void delegateInputToCurrentState();

  std::unique_ptr<Module<IRenderer>> module;
  IRenderer *renderer = nullptr;
  std::shared_ptr<ecs::World> m_world;
  std::shared_ptr<INetworkManager> m_networkManager;
  ecs::Entity m_inputEntity{0};
  bool isRunning = false;
  GameState currentState = GameState::MENU;
  std::unique_ptr<Menu> menu;
  std::unique_ptr<LobbyRoomState> lobbyRoomState;
  std::unique_ptr<PlayingState> playingState;
};
