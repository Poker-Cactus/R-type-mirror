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

Game::Game() : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU) {}

Game::~Game()
{
  shutdown();
}

bool Game::init()
{
  try {
#ifdef __APPLE__
    module = std::make_unique<Module<IRenderer>>("./build/libs/sdl2_module.dylib", "createRenderer", "destroyRenderer");
#else
    module = std::make_unique<Module<IRenderer>>("./build/libs/sdl2_module.so", "createRenderer", "destroyRenderer");
#endif
    renderer = module->create();

    if (renderer == nullptr) {
      std::cerr << "[Game::init] ERROR: Renderer is null" << '\n';
      return false;
    }
    renderer->setWindowTitle("ChaD");
    renderer->setFullscreen(true);

    menu = std::make_unique<Menu>(renderer);
    menu->init();

    m_world = std::make_shared<ecs::World>();
    auto asioClient = std::make_shared<AsioClient>("127.0.0.1", "4242");
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
    m_world->registerSystem<ClientNetworkReceiveSystem>(m_networkManager);

    playingState = std::make_unique<PlayingState>(renderer, m_world);
    if (!playingState->init()) {
      std::cerr << "Failed to initialize playing state" << '\n';
      return false;
    }
    isRunning = true;
    return true;
  } catch (const std::exception &e) {
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

void Game::processInput()
{
  if (renderer != nullptr && !renderer->pollEvents()) {
    std::cout << "[Game] pollEvents() returned false - shutting down" << '\n';
    isRunning = false;
    return;
  }

  handleMenuStateInput();
  handleLobbyRoomTransition();
  handlePlayingStateInput();
  delegateInputToCurrentState();
}

void Game::handleMenuStateInput()
{
  if (menu && currentState == GameState::MENU && menu->getState() == MenuState::EXIT) {
    isRunning = false;
  }
}

void Game::handleLobbyRoomTransition()
{
  if (!menu || currentState != GameState::MENU || !menu->shouldStartGame()) {
    return;
  }

  currentState = GameState::LOBBY_ROOM;

  if (!lobbyRoomState) {
    lobbyRoomState = std::make_unique<LobbyRoomState>(renderer, m_world);
    if (!lobbyRoomState->init()) {
      std::cerr << "[Game] Failed to initialize lobby room state" << '\n';
      lobbyRoomState.reset();
      currentState = GameState::MENU;
    }
  }
}

void Game::handlePlayingStateInput()
{
  if (currentState != GameState::PLAYING) {
    return;
  }

  if (playingState && playingState->shouldReturnToMenu()) {
    std::cout << "[Game] Player died - returning to menu" << '\n';
    currentState = GameState::MENU;
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
