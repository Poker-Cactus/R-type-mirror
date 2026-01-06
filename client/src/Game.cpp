/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game implementation
*/

#include "Game.hpp"
#include "../interface/KeyCodes.hpp"
#include "Menu/MenuState.hpp"
#include <cstddef>
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>

Game::Game()
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost("127.0.0.1"),
      m_serverPort("4242")
{
}

Game::Game(const std::string &host, const std::string &port)
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost(host),
      m_serverPort(port)
{
}

Game::~Game()
{
  shutdown();
}

bool Game::init()
{
  try {
    // Try multiple paths for the SDL2 module
    const char *modulePaths[] = {
#ifdef _WIN32
      "sdl2_module.dll", "libs/sdl2_module.dll", "./build/libs/sdl2_module.dll"
#elif defined(__APPLE__)
      "sdl2_module.dylib", "libs/sdl2_module.dylib", "./build/libs/sdl2_module.dylib"
#else
      "sdl2_module.so", "libs/sdl2_module.so", "./build/libs/sdl2_module.so"
#endif
    };

    bool moduleLoaded = false;
    for (const char *path : modulePaths) {
      try {
        module = std::make_unique<Module<IRenderer>>(path, "createRenderer", "destroyRenderer");
        std::cout << "[Game::init] Loaded SDL2 module from: " << path << std::endl;
        moduleLoaded = true;
        break;
      } catch (const std::exception &e) {
        // Try next path
        continue;
      }
    }

    if (!moduleLoaded) {
      std::cerr << "[Game::init] ERROR: Could not find SDL2 module in any known location" << std::endl;
      return false;
    }

    renderer = module->create();

    if (renderer == nullptr) {
      std::cerr << "[Game::init] ERROR: Renderer is null" << '\n';
      return false;
    }
    renderer->setWindowTitle("ChaD");

    // Start the game in fullscreen by default
    try {
      renderer->setFullscreen(true);
    } catch (const std::exception &e) {
      std::cerr << "[Game::init] Warning: failed to set fullscreen: " << e.what() << '\n';
    }

    menu = std::make_unique<Menu>(renderer);
    menu->init();

    m_world = std::make_shared<ecs::World>();
    auto asioClient = std::make_shared<AsioClient>(m_serverHost, m_serverPort);
    asioClient->start();
    {
      auto endpoint = asioClient->getServerEndpoint();
      std::cout << "[Client] Networking to " << endpoint.address().to_string() << ":" << endpoint.port() << '\n';
    }
    {
      const auto serialized = asioClient->getPacketHandler()->serialize("PING");
      asioClient->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
    }

    {
      nlohmann::json viewport;
      viewport["type"] = "viewport";
      viewport["width"] = static_cast<std::uint32_t>(renderer->getWindowWidth());
      viewport["height"] = static_cast<std::uint32_t>(renderer->getWindowHeight());
      const std::string jsonStr = viewport.dump();
      const auto serialized = asioClient->getPacketHandler()->serialize(jsonStr);
      asioClient->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
      std::cout << "[Client] Sent viewport " << viewport["width"] << "x" << viewport["height"] << '\n';
    }

    m_networkManager = asioClient;
    m_world->registerSystem<NetworkSendSystem>(m_networkManager);
    auto *networkReceiveSystem = &m_world->registerSystem<ClientNetworkReceiveSystem>(m_networkManager);

    if (networkReceiveSystem != nullptr) {
      networkReceiveSystem->setGameStartedCallback([this]() {
        std::cout << "[Game] Game started callback triggered - transitioning to PLAYING" << '\n';
        // Ensure the playing state exists and is initialized (recreate after death)
        if (!this->playingState) {
          this->playingState = std::make_unique<PlayingState>(this->renderer, this->m_world);
          if (!this->playingState->init()) {
            std::cerr << "[Game] Failed to initialize playing state on game_started" << '\n';
            // Fallback to menu if we cannot initialize rendering state
            this->currentState = GameState::MENU;
            if (this->menu)
              this->menu->setState(MenuState::MAIN_MENU);
            return;
          }
        }

        this->currentState = GameState::PLAYING;
        // Send current viewport to server immediately after the game starts
        // so the server records the correct client viewport for the playing session.
        this->sendViewportToServer();
      });
    }

    playingState = std::make_unique<PlayingState>(renderer, m_world);
    if (!playingState->init()) {
      std::cerr << "Failed to initialize playing state" << '\n';
      return false;
    }
    isRunning = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[Game::init] EXCEPTION: " << e.what() << std::endl;
    return false;
  }
}

void Game::run()
{
  if (!isRunning || renderer == nullptr) {
    return;
  }

  while (isRunning) {
    processInput();

    float deltaTime = renderer->getDeltaTime();
    update(deltaTime);

    render();
  }
}

void Game::shutdown()
{
  // Notify server that we're leaving before shutting down
  sendLeaveToServer();

  if (menu) {
    menu->cleanup();
    menu.reset();
  }

  if (lobbyRoomState) {
    lobbyRoomState->cleanup();
    lobbyRoomState.reset();
  }

  if (playingState) {
    playingState->cleanup();
    playingState.reset();
  }

  if (m_networkManager) {
    m_networkManager->stop();
    m_networkManager.reset();
  }
  m_world.reset();
  if (module && renderer != nullptr) {
    module->destroy(renderer);
    renderer = nullptr;
  }
  module.reset();
  isRunning = false;
}

void Game::sendLeaveToServer()
{
  if (!m_networkManager) {
    return;
  }

  std::cout << "[Game] Sending leave_lobby to server before shutdown" << '\n';

  nlohmann::json message;
  message["type"] = "leave_lobby";

  std::string jsonStr = message.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
}

void Game::sendViewportToServer()
{
  if (!m_networkManager || renderer == nullptr) {
    return;
  }

  nlohmann::json viewport;
  viewport["type"] = "viewport";
  viewport["width"] = static_cast<std::uint32_t>(renderer->getWindowWidth());
  viewport["height"] = static_cast<std::uint32_t>(renderer->getWindowHeight());

  std::string jsonStr = viewport.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

  std::cout << "[Game] Sent viewport update: " << viewport["width"] << "x" << viewport["height"] << '\n';
}

void Game::processInput()
{
  if (renderer != nullptr && !renderer->pollEvents()) {
    // Don't allow immediate close in lobby - give network time to send messages
    constexpr float LOBBY_GRACE_PERIOD = 0.5F;
    if (currentState == GameState::LOBBY_ROOM && m_lobbyStateTime < LOBBY_GRACE_PERIOD) {
      std::cout << "[Game] Ignoring close request - lobby just started (" << m_lobbyStateTime << "s)" << '\n';
      return;
    }
    std::cout << "[Game] pollEvents() returned false - shutting down" << '\n';
    isRunning = false;
    return;
  }

  // Ignore ESC in lobby room to prevent accidental closures
  if (currentState == GameState::LOBBY_ROOM && renderer != nullptr && renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE)) {
    std::cout << "[Game] ESC pressed in lobby - ignoring (use quit from menu to exit)" << '\n';
    return;
  }

  if (renderer != nullptr && renderer->isKeyJustPressed(KeyCode::KEY_M)) {
    bool currentFullscreen = renderer->isFullscreen();
    renderer->setFullscreen(!currentFullscreen);
    std::cout << "[Game] Toggled fullscreen: " << (!currentFullscreen ? "ON" : "OFF") << '\n';

    // Send updated viewport to server
    sendViewportToServer();
  }

  handleMenuStateInput();
  handleLobbyRoomTransition();
  handleLobbyRoomStateInput();
  handlePlayingStateInput();
  delegateInputToCurrentState();
}

void Game::handleMenuStateInput()
{
  if (menu && currentState == GameState::MENU && menu->getState() == MenuState::EXIT) {
    isRunning = false;
  }
}

void Game::handleLobbyRoomStateInput()
{
  if (currentState != GameState::LOBBY_ROOM) {
    return;
  }

  if (lobbyRoomState && lobbyRoomState->shouldReturnToMenu()) {
    std::cout << "[Game] Returning from lobby to menu" << '\n';
    currentState = GameState::MENU;
    menu->setState(MenuState::LOBBY);
    lobbyRoomState.reset();
    return;
  }
}

void Game::handleLobbyRoomTransition()
{
  if (!menu) {
    return;
  }

  if (currentState != GameState::MENU) {
    return;
  }

  if (!menu->shouldStartGame()) {
    return;
  }

  // Get lobby info from menu
  const bool isCreating = menu->isCreatingLobby();
  const std::string lobbyCode = menu->getLobbyCodeToJoin();

  std::cout << "[Game] Transitioning from MENU to LOBBY_ROOM" << '\n';
  std::cout << "[Game] Creating: " << (isCreating ? "yes" : "no");
  if (!isCreating) {
    std::cout << ", Code: " << lobbyCode;
  }
  std::cout << '\n';

  // Reset the menu flag
  menu->resetLobbySelection();

  currentState = GameState::LOBBY_ROOM;
  m_lobbyStateTime = 0.0F;

  // Create lobby room state if needed
  if (!lobbyRoomState) {
    lobbyRoomState = std::make_unique<LobbyRoomState>(renderer, m_world, m_networkManager);
    if (!lobbyRoomState->init()) {
      std::cerr << "[Game] Failed to initialize lobby room state" << '\n';
      lobbyRoomState.reset();
      currentState = GameState::MENU;
      return;
    }
  }

  // Set the lobby mode (create or join)
  lobbyRoomState->setLobbyMode(isCreating, lobbyCode);

  // Connect network callbacks to lobby state
  if (auto *networkReceiveSystem = m_world->getSystem<ClientNetworkReceiveSystem>()) {
    networkReceiveSystem->setLobbyJoinedCallback([this](const std::string &code) {
      if (lobbyRoomState) {
        lobbyRoomState->onLobbyJoined(code);
      }
    });

    networkReceiveSystem->setLobbyStateCallback([this](const std::string &code, int playerCount) {
      if (lobbyRoomState) {
        lobbyRoomState->onLobbyState(code, playerCount);
      }
    });

    networkReceiveSystem->setErrorCallback([this](const std::string &errorMsg) {
      if (lobbyRoomState) {
        lobbyRoomState->onError(errorMsg);
      }
    });
    // Player-dead: server told us our player is dead and we should return to menu
    networkReceiveSystem->setPlayerDeadCallback([this](UNUSED const nlohmann::json &msg) {
      std::cout << "[Game] Received player_dead from server - returning to menu" << std::endl;

      // Stop accepting snapshots
      if (auto *netRec = m_world->getSystem<ClientNetworkReceiveSystem>()) {
        netRec->setAcceptSnapshots(false);
      }

      // Inform server we're leaving (best effort)
      sendLeaveToServer();

      // Clean up playing state
      if (playingState) {
        playingState->cleanup();
        playingState.reset();
      }

      // Clear world entities to stop any further rendering or AI
      if (m_world) {
        try {
          ecs::ComponentSignature emptySig; // matches all
          std::vector<ecs::Entity> allEntities;
          m_world->getEntitiesWithSignature(emptySig, allEntities);
          for (auto e : allEntities) {
            if (m_world->isAlive(e)) {
              m_world->destroyEntity(e);
            }
          }
        } catch (const std::exception &e) {
          std::cerr << "[Game] Error clearing world on player_dead: " << e.what() << '\n';
        }
      }

      // Transition to main menu
      currentState = GameState::MENU;
      if (menu) {
        menu->setState(MenuState::MAIN_MENU);
      }
    });

    // Optional: handle server acknowledgement when lobby leave is processed
    networkReceiveSystem->setLobbyLeftCallback([this]() {
      std::cout << "[Game] Server acknowledged lobby_left" << std::endl;
      // If we're in lobby UI, ensure it returns to main menu
      if (lobbyRoomState) {
        lobbyRoomState.reset();
        currentState = GameState::MENU;
        if (menu)
          menu->setState(MenuState::MAIN_MENU);
      }
    });
  }
}

void Game::handlePlayingStateInput()
{
  if (currentState != GameState::PLAYING) {
    return;
  }

  if (playingState && playingState->shouldReturnToMenu()) {
    std::cout << "[Game] Player died - returning to menu" << '\n';
    // Notify server that we're leaving the lobby/game
    sendLeaveToServer();

    // Tell network receive system to stop accepting snapshots immediately
    if (m_world) {
      if (auto *netRecv = m_world->getSystem<ClientNetworkReceiveSystem>()) {
        netRecv->setAcceptSnapshots(false);
      }
    }

    // Clean up playing state resources
    playingState->cleanup();
    playingState.reset();

    // Also clear all entities from the client world to avoid stale data
    if (m_world) {
      try {
        ecs::ComponentSignature emptySig; // matches all
        std::vector<ecs::Entity> allEntities;
        m_world->getEntitiesWithSignature(emptySig, allEntities);
        for (auto e : allEntities) {
          if (m_world->isAlive(e)) {
            m_world->destroyEntity(e);
          }
        }
      } catch (const std::exception &e) {
        std::cerr << "[Game] Error clearing world on death: " << e.what() << '\n';
      }
    }

    // Return to main menu
    currentState = GameState::MENU;
    if (menu) {
      menu->setState(MenuState::MAIN_MENU);
    }
    return;
  }

  if (playingState) {
    static int logCounter = 0;
    constexpr int LOG_INTERVAL = 120;
    if (++logCounter % LOG_INTERVAL == 0) {
      std::cout << "[Game] Playing state active, shouldReturnToMenu = " << playingState->shouldReturnToMenu() << '\n';
    }
  }

  updatePlayerInput();
}

void Game::updatePlayerInput()
{
  ensureInputEntity();

  if (!m_world || m_inputEntity == 0 || !m_world->hasComponent<ecs::Input>(m_inputEntity)) {
    return;
  }

  auto &input = m_world->getComponent<ecs::Input>(m_inputEntity);
  input.up = renderer->isKeyPressed(KeyCode::KEY_UP) || renderer->isKeyPressed(KeyCode::KEY_W) ||
    renderer->isKeyPressed(KeyCode::KEY_Z);
  input.down = renderer->isKeyPressed(KeyCode::KEY_DOWN) || renderer->isKeyPressed(KeyCode::KEY_S);
  input.left = renderer->isKeyPressed(KeyCode::KEY_LEFT) || renderer->isKeyPressed(KeyCode::KEY_A) ||
    renderer->isKeyPressed(KeyCode::KEY_Q);
  input.right = renderer->isKeyPressed(KeyCode::KEY_RIGHT) || renderer->isKeyPressed(KeyCode::KEY_D);
  input.shoot = renderer->isKeyPressed(KeyCode::KEY_SPACE);
}

void Game::delegateInputToCurrentState()
{
  switch (currentState) {
  case GameState::MENU:
    if (menu) {
      menu->processInput();
    }
    break;
  case GameState::LOBBY_ROOM:
    if (lobbyRoomState) {
      lobbyRoomState->processInput();
    }
    break;
  case GameState::PLAYING:
    if (playingState) {
      playingState->processInput();
    }
    break;
  case GameState::PAUSED:
    // TODO: handle pause input
    break;
  }
}

void Game::ensureInputEntity()
{
  if (!m_world) {
    return;
  }

  if (m_inputEntity != 0 && m_world->isAlive(m_inputEntity)) {
    if (!m_world->hasComponent<ecs::Input>(m_inputEntity)) {
      m_world->addComponent(m_inputEntity, ecs::Input{});
    }
    return;
  }

  m_inputEntity = m_world->createEntity();
  m_world->addComponent(m_inputEntity, ecs::Input{});
}

void Game::update(float deltaTime)
{
  if (m_world) {
    m_world->update(deltaTime);
  }

  // Track time in lobby state
  if (currentState == GameState::LOBBY_ROOM) {
    m_lobbyStateTime += deltaTime;
  }

  switch (currentState) {
  case GameState::MENU:
    break;
  case GameState::LOBBY_ROOM:
    if (lobbyRoomState) {
      lobbyRoomState->update(deltaTime);
    }
    break;
  case GameState::PLAYING:
    if (playingState) {
      playingState->update(deltaTime);
    }
    break;
  case GameState::PAUSED:
    break;
  }
}

void Game::render()
{
  constexpr std::uint8_t ALPHA_OPAQUE = 255;
  renderer->clear({0, 0, 0, ALPHA_OPAQUE});

  switch (currentState) {
  case GameState::MENU:
    if (menu) {
      menu->render();
    }
    break;
  case GameState::LOBBY_ROOM:
    if (lobbyRoomState) {
      lobbyRoomState->render();
    }
    break;
  case GameState::PLAYING:
  case GameState::PAUSED:
    if (playingState) {
      playingState->render();
    }
    break;
  }

  renderer->present();
}

void Game::setState(GameState newState)
{
  currentState = newState;
}

Game::GameState Game::getState() const
{
  return currentState;
}
