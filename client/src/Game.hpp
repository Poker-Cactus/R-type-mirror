#pragma once
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../ModuleLoader.hpp"
#include "../interface/IRenderer.hpp"
#include "Menu.hpp"
#include "PlayingState.hpp"
#include <memory>

namespace ecs
{
class World;
}

class INetworkManager;
class Game
{
public:
  enum class GameState : std::uint8_t { MENU, PLAYING, PAUSED };

  Game();
  Game(const std::string &host, const std::string &port);
  ~Game();

  bool init();
  void run();
  void shutdown();

  void setState(GameState newState);
  GameState getState() const;

private:
  void processInput();
  void update(float deltaTime);
  void render();
  void ensureInputEntity();

  std::unique_ptr<Module<IRenderer>> module;
  IRenderer *renderer = nullptr;

  std::shared_ptr<ecs::World> m_world;
  std::shared_ptr<INetworkManager> m_networkManager;
  std::string m_serverHost;
  std::string m_serverPort;
  ecs::Entity m_inputEntity{0};
  bool isRunning = false;
  GameState currentState = GameState::MENU;
  std::unique_ptr<Menu> menu;
  std::unique_ptr<PlayingState> playingState;
};
