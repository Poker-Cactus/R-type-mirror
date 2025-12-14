/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/AsioClient.hpp"
#include "../include/systems/NetworkReceiveSystem.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include "../interface/KeyCodes.hpp"
#include "../ModuleLoader.hpp"
#include "../interface/IRenderer.hpp"
#include "Menu.hpp"
#include "Menu/MenuState.hpp"
#include "PlayingState.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <span>
#include <string>

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
      return false;
    }
    renderer->setWindowTitle("ChaD");
    renderer->setFullscreen(true);

    menu = std::make_unique<Menu>(renderer);
    menu->init();

    // Networking + ECS: run network systems in the same loop as the graphical game.
    m_world = std::make_shared<ecs::World>();
    auto asioClient = std::make_shared<AsioClient>("127.0.0.1", "4242");
    asioClient->start();
    {
      auto ep = asioClient->getServerEndpoint();
      std::cout << "[Client] Networking to " << ep.address().to_string() << ":" << ep.port() << std::endl;
    }
    // Ensure the server learns our UDP endpoint.
    {
      const auto serialized = asioClient->getPacketHandler()->serialize("PING");
      asioClient->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
    }

    // Send our current viewport size so the server can clamp us correctly.
    {
      nlohmann::json viewport;
      viewport["type"] = "viewport";
      viewport["width"] = static_cast<std::uint32_t>(renderer->getWindowWidth());
      viewport["height"] = static_cast<std::uint32_t>(renderer->getWindowHeight());
      const std::string jsonStr = viewport.dump();
      const auto serialized = asioClient->getPacketHandler()->serialize(jsonStr);
      asioClient->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
      std::cout << "[Client] Sent viewport " << viewport["width"] << "x" << viewport["height"] << std::endl;
    }

    m_networkManager = asioClient;
    m_world->registerSystem<NetworkSendSystem>(m_networkManager);
    m_world->registerSystem<ClientNetworkReceiveSystem>(m_networkManager);

    playingState = std::make_unique<PlayingState>(renderer, m_world);
    if (!playingState->init()) {
      std::cerr << "Failed to initialize playing state" << std::endl;
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

    float dt = renderer->getDeltaTime();
    update(dt);

    render();
  }
}

void Game::shutdown()
{
  if (menu) {
    menu->cleanup();
    menu.reset();
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
  if (module && renderer) {
    module->destroy(renderer);
    renderer = nullptr;
  }
  module.reset();
  isRunning = false;
}

void Game::processInput()
{
  if (!renderer->pollEvents()) {
    isRunning = false;
  }
  // if (this->menu && this->currentState == GameState::MENU)
  //     this->menu->processInput();
  if (this->menu && this->currentState == GameState::MENU && this->menu->getState() == MenuState::EXIT)
    this->isRunning = false;

  // Vérifier si le menu veut lancer le jeu
  if (this->menu && this->currentState == GameState::MENU && this->menu->shouldStartGame()) {
    this->currentState = GameState::PLAYING;
  }

  if (this->currentState == GameState::PLAYING) {
    // Check if player died and should return to menu
    if (playingState && playingState->shouldReturnToMenu()) {
      std::cout << "[Game] Player died - returning to menu" << std::endl;
      this->currentState = GameState::MENU;
    } else if (playingState) {
      // Debug log to check state
      static int logCounter = 0;
      if (++logCounter % 120 == 0) { // Log every ~2 seconds
        std::cout << "[Game] Playing state active, shouldReturnToMenu = " << playingState->shouldReturnToMenu()
                  << std::endl;
      }
    }

    ensureInputEntity();
    if (m_world && m_inputEntity != 0 && m_world->hasComponent<ecs::Input>(m_inputEntity)) {
      auto &input = m_world->getComponent<ecs::Input>(m_inputEntity);
      input.up = renderer->isKeyPressed(KeyCode::KEY_UP) || renderer->isKeyPressed(KeyCode::KEY_W) ||
        renderer->isKeyPressed(KeyCode::KEY_Z);
      input.down = renderer->isKeyPressed(KeyCode::KEY_DOWN) || renderer->isKeyPressed(KeyCode::KEY_S);
      input.left = renderer->isKeyPressed(KeyCode::KEY_LEFT) || renderer->isKeyPressed(KeyCode::KEY_A) ||
        renderer->isKeyPressed(KeyCode::KEY_Q);
      input.right = renderer->isKeyPressed(KeyCode::KEY_RIGHT) || renderer->isKeyPressed(KeyCode::KEY_D);
      input.shoot = renderer->isKeyPressed(KeyCode::KEY_SPACE);
    }
  }

  switch (currentState) {
  case GameState::MENU:
    if (menu) {
      menu->processInput();
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

void Game::update(float dt)
{
  if (m_world) {
    m_world->update(dt);
  }
  switch (currentState) {
  case GameState::MENU:
    // Menu est statique, pas besoin d'update
    break;
  case GameState::PLAYING:
    if (playingState) {
      playingState->update(dt);
    }
    break;
  case GameState::PAUSED:
    // Pause ne met pas à jour le jeu
    break;
  }
}

void Game::render()
{
  renderer->clear({0, 0, 0, 255});

  switch (currentState) {
  case GameState::MENU:
    if (menu) {
      menu->render();
    }
    break;
  case GameState::PLAYING:
    if (playingState) {
      playingState->render();
    }
    break;
  case GameState::PAUSED:
    // TODO: render pause menu
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
