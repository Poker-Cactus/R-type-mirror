/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include "../../client/interface/KeyCodes.hpp"
#include <chrono>
#include <exception>
#include <vector>

Game::Game()
    : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU), m_serverHost("127.0.0.1"),
      m_serverPort("4243"), m_rendererType("sfml")
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

Game::~Game()
{
  shutdown();
}

bool Game::init()
{
  std::vector<std::string> modulePaths;
  std::vector<std::string> basePaths = {
    "../../libs/sdl2_module.so",   "../../libs/sfml_module.so", // From build/flappyBird/client/
    "./build/libs/sdl2_module.so", "./build/libs/sfml_module.so", // From project root
    "libs/sdl2_module.so",         "libs/sfml_module.so",         "sdl2_module.so", "sfml_module.so"};

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

  // Initialize network manager
  m_networkManager = std::make_shared<AsioClient>(m_serverHost, m_serverPort);
  std::cout << "[Game::init] Connected to server at " << m_serverHost << ":" << m_serverPort << std::endl;

  // Initialize ECS world
  m_world = std::make_shared<ecs::World>();
  std::cout << "[Game::init] ECS World initialized" << std::endl;

  try {
    menu = std::make_unique<Menu>(renderer);
    if (!menu->init()) {
      std::cerr << "[Game] ERROR: Menu initialization failed" << std::endl;
      return false;
    }
    std::cout << "[Game] Menu initialized successfully" << std::endl;
  } catch (std::exception &error) {
    std::cerr << "[Game] ERROR: Menu exception: " << error.what() << std::endl;
    return false;
  }

  try {
    lobby = std::make_unique<Lobby>(renderer);
    if (!lobby->init()) {
      std::cerr << "[Game] ERROR: Lobby initialization failed" << std::endl;
      return false;
    }
    std::cout << "[Game] Lobby initialized successfully" << std::endl;
  } catch (std::exception &error) {
    std::cerr << "[Game] ERROR: Lobby exception: " << error.what() << std::endl;
    return false;
  }

  isRunning = true;
  return true;
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

void Game::run()
{
  if (!isRunning || renderer == nullptr) {
    return;
  }

  std::cout << "[Game] Starting game loop" << std::endl;
  auto lastTime = std::chrono::high_resolution_clock::now();

  while (isRunning) {
    processInput();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Poll window events
    if (!renderer->pollEvents()) {
      isRunning = false;
      break;
    }

    update(deltaTime);
    render();
  }

  std::cout << "[Game] Game loop ended" << std::endl;
}

void Game::processInput()
{
  if (renderer == nullptr)
    return;

  switch (currentState) {
  case GameState::MENU: {
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      currentState = GameState::LOBBY_ROOM;
      std::cout << "[Game] Entering lobby..." << std::endl;
    }
    break;
  }
  case GameState::LOBBY_ROOM: {
    if (!lobby) {
      std::cerr << "[Game] ERROR: Lobby is null!" << std::endl;
      currentState = GameState::MENU;
      break;
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_X)) {
      lobby->setReady(!lobby->isReady());
      std::cout << "[Game] Ready status: " << (lobby->isReady() ? "READY" : "NOT READY") << std::endl;
    }
    if (lobby->isReady() && renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      currentState = GameState::PLAYING;
      std::cout << "[Game] Starting game..." << std::endl;
    }
    break;
  }
  case GameState::PLAYING: {
    // Game input handling will go here
    break;
  }
  default:
    break;
  }
}

void Game::update(float deltaTime)
{
  // Update menu animation
  if (currentState == GameState::MENU && menu) {
    menu->update(deltaTime);
  }

  // Update lobby animation
  if (currentState == GameState::LOBBY_ROOM && lobby) {
    lobby->update(deltaTime);
  }

  // Update ECS world - systems will process entities
  if (m_world) {
    m_world->update(deltaTime);
  }
}

void Game::render()
{
  if (!renderer) {
    return;
  }

  renderer->clear();

  switch (currentState) {
  case GameState::MENU: {
    if (menu) {
      menu->render();
    }
    break;
  }
  case GameState::LOBBY_ROOM: {
    if (lobby) {
      lobby->render();
    }
    break;
  }
  case GameState::PLAYING: {
    // Game rendering will go here
    break;
  }
  default:
    break;
  }

  // ECS render systems will draw entities here
  renderer->present();
}