/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include <vector>

Game::Game()
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost("127.0.0.1"),
      m_serverPort("4242"), m_rendererType("sfml")
{
}

Game::Game(const std::string &host, const std::string &port)
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost(host),
      m_serverPort(port), m_rendererType("sfml")
{
}

Game::Game(const std::string &host, const std::string &port, const std::string &rendererType)
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost(host),
      m_serverPort(port), m_rendererType(rendererType)
{
}

Game::~Game() {
    shutdown();
}

bool Game::init() {
    std::vector<std::string> modulePaths;
    std::vector<std::string> basePaths = { "sdl2_module.so", "libs/sdl2_module.so", "./build/libs/sdl2_module.so",
      "sfml_module.so", "libs/sfml_module.so", "./build/libs/sfml_module.so",
      "../libs/sdl2_module.so", "../libs/sfml_module.so" };

      // Prioritize the requested renderer type
    for (const auto &path : basePaths) {
      if ((m_rendererType == "sdl2" && path.find("sdl2_module") != std::string::npos) ||
          (m_rendererType == "sfml" && path.find("sfml_module") != std::string::npos)) {
        modulePaths.push_back(path);
      }
    }

    // Add the other renderer as fallback
    for (const auto &path : basePaths) {
      if ((m_rendererType == "sdl2" && path.find("sfml_module") != std::string::npos) ||
          (m_rendererType == "sfml" && path.find("sdl2_module") != std::string::npos)) {
        modulePaths.push_back(path);
      }
    }

    bool moduleLoaded = false;
    for (const std::string &path : modulePaths) {
      try {
        module = std::make_unique<Module<IRenderer>>(path.c_str(), "createRenderer", "destroyRenderer");
        std::cout << "[Game::init] Loaded " << m_rendererType << " module from: " << path << std::endl;
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

    renderer = std::shared_ptr<IRenderer>(module->create(), [this](IRenderer *ptr) {
      if (module) {
        module->destroy(ptr);
      }
    });

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
}

void Game::shutdown()
{
  // Save settings before shutting down
//   settings.saveToFile();

  // Notify server that we're leaving before shutting down
  sendLeaveToServer();

//   if (menu) {
//     menu->cleanup();
//     menu.reset();
//     // Renderer will be automatically destroyed by shared_ptr when all references are gone
//   }

  if (m_networkManager) {
    m_networkManager->stop();
    m_networkManager.reset();
  }
  m_world.reset();

  // Reset renderer - custom deleter will call module->destroy
  renderer.reset();
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

void Game::run() {
    if (!isRunning || renderer == nullptr) {
    return;
  }

  while (isRunning) {
    // processInput();

    // float deltaTime = renderer->getDeltaTime();
    // update(deltaTime);

    // render();
  }
}