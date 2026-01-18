/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game implementation
*/

#include "Game.hpp"
#include "../interface/IColorBlindSupport.hpp"
#include "../interface/KeyCodes.hpp"
#include "Menu/MenuState.hpp"
#include "Settings.hpp"
#include <cstddef>
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>
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

Game::~Game()
{
  shutdown();
}

bool Game::init()
{
  try {
    // Load settings from file
    settings.loadFromFile();

    // Try multiple paths for the renderer modules
    std::vector<std::string> modulePaths;

    // Add paths for both SDL2 and SFML modules
    std::vector<std::string> basePaths = {
#ifdef _WIN32
      "sdl2_module.dll",      "libs/sdl2_module.dll",        "./build/libs/sdl2_module.dll", "sfml_module.dll",
      "libs/sfml_module.dll", "./build/libs/sfml_module.dll"
#elif defined(__APPLE__)
      "sdl2_module.dylib",      "libs/sdl2_module.dylib",        "./build/libs/sdl2_module.dylib", "sfml_module.dylib",
      "libs/sfml_module.dylib", "./build/libs/sfml_module.dylib"
#else
      "sdl2_module.so",         "libs/sdl2_module.so",   "./build/libs/sdl2_module.so",
      "sfml_module.so",         "libs/sfml_module.so",   "./build/libs/sfml_module.so",
      "../libs/sdl2_module.so", "../libs/sfml_module.so"
#endif
    };

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

    menu = std::make_unique<Menu>(renderer, settings);
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
      // End-screen handler: show end-screen payload and wait for BACKSPACE
      networkReceiveSystem->setLobbyEndCallback([this](const nlohmann::json &msg) {
        std::cout << "[Game] Lobby end received, showing end-screen" << std::endl;
        // Store payload for render/interaction
        this->m_endScreenPayload = msg;
        this->m_showEndScreen = true;

        // If score for this client exists, save it locally to highscores
        if (msg.contains("scores") && menu) {
          auto &scores = msg["scores"];
          for (const auto &entry : scores) {
            if (entry.contains("client_id") && entry.contains("score")) {
              int clientId = entry["client_id"];
              int score = entry["score"];
              // If this is our client id, persist to highscores
              // We don't know our assigned client id here, so rely on Menu->Highscore addition when appropriate
            }
          }
        }
      });
      networkReceiveSystem->setGameStartedCallback([this]() {
        std::cout << "[Game] Game started callback triggered - transitioning to PLAYING" << '\n';
        // Ensure the playing state exists and is initialized (recreate after death)
        if (!this->playingState) {
          this->playingState =
            std::make_unique<PlayingState>(this->renderer, this->m_world, this->settings, this->m_networkManager);
          if (!this->playingState->init()) {
            std::cerr << "[Game] Failed to initialize playing state on game_started" << '\n';
            // Fallback to menu if we cannot initialize rendering state
            this->currentState = GameState::MENU;
            if (this->menu)
              this->menu->setState(MenuState::MAIN_MENU);
            return;
          }
          // Set solo mode if lobby room state exists
          if (this->lobbyRoomState) {
            this->playingState->setSoloMode(this->lobbyRoomState->isSolo());
          }
        }

        this->currentState = GameState::PLAYING;
        // Send current viewport to server immediately after the game starts
        // so the server records the correct client viewport for the playing session.
        this->sendViewportToServer();
      });

      // Set chat message callback to display incoming messages
      networkReceiveSystem->setChatMessageCallback(
        [this](const std::string &sender, const std::string &content, std::uint32_t senderId) {
          if (this->m_chatUI) {
            this->m_chatUI->addMessage(sender, content, false, senderId);
          }
        });
    }

    playingState = std::make_unique<PlayingState>(renderer, m_world, settings, m_networkManager);
    if (!playingState->init()) {
      std::cerr << "Failed to initialize playing state" << '\n';
      return false;
    }

    // Initialize chat UI
    m_chatUI = std::make_unique<ChatUI>(renderer);
    if (!m_chatUI->init()) {
      std::cerr << "[Game] Warning: Failed to initialize chat UI - disabling chat" << '\n';
      m_chatUI.reset(); // Disable chat completely if init fails
    } else {
      m_chatUI->setLocalUsername(settings.username);
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
  // Reset color blind mode to normal when shutting down (only if renderer supports it)
  if (renderer) {
    auto *colorBlindSupport = dynamic_cast<IColorBlindSupport *>(renderer.get());
    if (colorBlindSupport) {
      colorBlindSupport->setColorBlindMode(ColorBlindMode::NONE);
    }
  }

  // Save settings before shutting down
  settings.saveToFile();

  // Notify server that we're leaving before shutting down
  sendLeaveToServer();

  if (lobbyRoomState) {
    lobbyRoomState->cleanup();
    lobbyRoomState.reset();
  }

  if (playingState) {
    playingState->cleanup();
    playingState.reset();
  }

  if (menu) {
    menu->cleanup();
    menu.reset();
    // Renderer will be automatically destroyed by shared_ptr when all references are gone
  }

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

void Game::renderEndScreen()
{
  if (!renderer || !m_showEndScreen) {
    return;
  }

  // Semi-transparent dark overlay
  const int windowWidth = renderer->getWindowWidth();
  const int windowHeight = renderer->getWindowHeight();
  renderer->drawRect(0, 0, windowWidth, windowHeight, {0, 0, 0, 200});

  // Panel dimensions
  const int panelWidth = windowWidth * 2 / 3;
  const int panelHeight = windowHeight * 2 / 3;
  const int panelX = (windowWidth - panelWidth) / 2;
  const int panelY = (windowHeight - panelHeight) / 2;

  // Draw panel background (filled)
  renderer->drawRect(panelX, panelY, panelWidth, panelHeight, {20, 20, 40, 255});
  // Draw panel border
  renderer->drawRectOutline(panelX, panelY, panelWidth, panelHeight, {100, 100, 200, 255});

  // Load font for end-screen (same as menu)
  auto font = renderer->loadFont("client/assets/font.opf/r-type.otf", 32);
  auto smallFont = renderer->loadFont("client/assets/font.opf/r-type.otf", 24);

  // Title
  const std::string title = "GAME OVER";
  int titleWidth = 0, titleHeight = 0;
  renderer->getTextSize(font, title, titleWidth, titleHeight);
  renderer->drawText(font, title, panelX + (panelWidth - titleWidth) / 2, panelY + 40, {255, 100, 100, 255});

  // Scores
  int currentY = panelY + 120;
  if (m_endScreenPayload.contains("scores") && m_endScreenPayload["scores"].is_array()) {
    const std::string scoresTitle = "SCORES";
    int scoresTitleW = 0, scoresTitleH = 0;
    renderer->getTextSize(smallFont, scoresTitle, scoresTitleW, scoresTitleH);
    renderer->drawText(smallFont, scoresTitle, panelX + (panelWidth - scoresTitleW) / 2, currentY,
                       {200, 200, 255, 255});
    currentY += 50;

    for (const auto &entry : m_endScreenPayload["scores"]) {
      if (entry.contains("score")) {
        int score = entry["score"];
        std::string displayName;
        if (entry.contains("name") && entry["name"].is_string()) {
          displayName = entry["name"].get<std::string>();
        } else if (entry.contains("client_id")) {
          displayName = "Player " + std::to_string(entry["client_id"].get<int>());
        } else {
          displayName = "Player";
        }
        std::string scoreText = displayName + "  " + std::to_string(score) + " points";
        int textW = 0, textH = 0;
        renderer->getTextSize(smallFont, scoreText, textW, textH);
        renderer->drawText(smallFont, scoreText, panelX + (panelWidth - textW) / 2, currentY, {255, 255, 255, 255});
        currentY += 40;
      }
    }
  }

  // Instructions
  currentY = panelY + panelHeight - 80;
  const std::string instructions = "Press BACKSPACE to return to menu";
  int instrW = 0, instrH = 0;
  renderer->getTextSize(smallFont, instructions, instrW, instrH);
  renderer->drawText(smallFont, instructions, panelX + (panelWidth - instrW) / 2, currentY, {150, 255, 150, 255});

  renderer->freeFont(font);
  renderer->freeFont(smallFont);
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

void Game::sendChatMessage(const std::string &message)
{
  if (!m_networkManager || message.empty()) {
    return;
  }

  nlohmann::json chatMsg;
  chatMsg["type"] = "chat_message";
  chatMsg["content"] = message;
  chatMsg["sender"] = settings.username;

  std::string jsonStr = chatMsg.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

  std::cout << "[Game] Sent chat message: " << message << '\n';

  // Add to local chat UI immediately with client ID
  if (m_chatUI && m_world) {
    std::uint32_t localClientId = 0;
    if (auto *sendSys = m_world->getSystem<NetworkSendSystem>()) {
      localClientId = sendSys->getClientId();
    }
    m_chatUI->addMessage(settings.username, message, false, localClientId);
  }
}

void Game::handleChatInput()
{
  if (!renderer || !m_chatUI) {
    return;
  }

  // Open chat with T key (only if not already open)
  if (renderer->isKeyJustPressed(KeyCode::KEY_T) && !m_chatUI->isVisible()) {
    m_chatUI->open();

    // Reset player input when opening chat to prevent unwanted movement
    if (m_world && m_inputEntity != 0 && m_world->hasComponent<ecs::Input>(m_inputEntity)) {
      auto &input = m_world->getComponent<ecs::Input>(m_inputEntity);
      input.up = false;
      input.down = false;
      input.left = false;
      input.right = false;
      input.shoot = false;
      input.chargedShoot = false;
      input.detach = false;
    }

    // Reset player animation to idle
    if (playingState) {
      playingState->resetPlayerAnimation();
    }
    return;
  }

  // Close chat with Escape key
  if (renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE) && m_chatUI->isVisible()) {
    m_chatUI->close();
    return;
  }

  // Process chat input if visible
  if (m_chatUI->isVisible()) {
    m_chatUI->processInput();

    // Check if user wants to send a message
    if (m_chatUI->hasMessageToSend()) {
      std::string message = m_chatUI->consumeMessage();
      sendChatMessage(message);
    }
  }
}

void Game::processInput()
{
  if (renderer != nullptr && !renderer->pollEvents()) {
    constexpr float LOBBY_GRACE_PERIOD = 0.5F;
    if (currentState == GameState::LOBBY_ROOM && m_lobbyStateTime < LOBBY_GRACE_PERIOD) {
      std::cout << "[Game] Ignoring close request - lobby just started (" << m_lobbyStateTime << "s)" << '\n';
      return;
    }
    std::cout << "[Game] pollEvents() returned false - shutting down" << '\n';
    isRunning = false;
    return;
  }

  // Handle ESC key - close chat if open, otherwise handle normally
  if (renderer != nullptr && renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE)) {
    if (m_chatUI && m_chatUI->isVisible()) {
      // Close chat instead of quitting
      m_chatUI->close();
      return;
    }
    // In menu, ESC can be used for navigation (handled by menu)
    if (currentState == GameState::MENU) {
      // Let menu handle ESC
    } else if (currentState == GameState::LOBBY_ROOM) {
      std::cout << "[Game] ESC pressed in lobby - ignoring (use quit from menu to exit)" << '\n';
      return;
    } else {
      // In other states, ESC quits the game
      std::cout << "[Game] ESC pressed - shutting down" << '\n';
      isRunning = false;
      return;
    }
  }

  // If end-screen is active, only accept BACKSPACE to leave
  if (m_showEndScreen) {
    if (renderer != nullptr && renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      // Tell server we've left the end-screen (so it can destroy lobby if everyone left)
      if (m_networkManager) {
        nlohmann::json msg;
        msg["type"] = "end_screen_left";
        const std::string jsonStr = msg.dump();
        const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);
        m_networkManager->send(
          std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
      }

      // Return to menu
      m_showEndScreen = false;
      currentState = GameState::MENU;
      if (menu) {
        menu->setState(MenuState::MAIN_MENU);
      }
    }
    return; // consume other inputs while end-screen is active
  }

  // Toggle fullscreen with M key (but not when editing profile)
  if (renderer != nullptr && renderer->isKeyJustPressed(KeyCode::KEY_M)) {
    // Don't toggle fullscreen if we're editing a username in the profile menu or chat is open
    if (!(currentState == GameState::MENU && menu && menu->getState() == MenuState::PROFILE &&
          menu->isProfileEditing()) &&
        !(m_chatUI && m_chatUI->isInputFocused())) {
      bool currentFullscreen = renderer->isFullscreen();
      renderer->setFullscreen(!currentFullscreen);
      std::cout << "[Game] Toggled fullscreen: " << (!currentFullscreen ? "ON" : "OFF") << '\n';

      // Send updated viewport to server
      sendViewportToServer();
    }
  }

  // Toggle chat with Ctrl+T
  handleChatInput();

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
  const Difficulty diff = menu->getLobbyMenu()->getSelectedDifficulty();
  const GameMode mode = menu->getLobbyMenu()->getSelectedGameMode();
  const AIDifficulty aiDiff = settings.aiDifficulty;
  const bool isSolo = menu->isSolo();

  std::cout << "[Game] Transitioning from MENU to LOBBY_ROOM" << '\n';
  std::cout << "[Game] Creating: " << (isCreating ? "yes" : "no");
  if (!isCreating) {
    std::cout << ", Code: " << lobbyCode;
  } else {
    std::cout << ", Difficulty: " << static_cast<int>(diff) << ", AI: " << static_cast<int>(aiDiff);
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

  // Provide settings pointer so LobbyRoomState can include username in requests
  lobbyRoomState->setSettings(&settings);

  // Set the lobby mode (create or join)
  lobbyRoomState->setLobbyMode(isCreating, lobbyCode, diff, isSolo, aiDiff, mode);

  // Connect network callbacks to lobby state
  if (auto *networkReceiveSystem = m_world->getSystem<ClientNetworkReceiveSystem>()) {
    networkReceiveSystem->setLobbyJoinedCallback([this](const std::string &code) {
      if (lobbyRoomState) {
        lobbyRoomState->onLobbyJoined(code);
      }
    });

    networkReceiveSystem->setLobbyStateCallback([this](const std::string &code, int playerCount, int spectatorCount) {
      if (lobbyRoomState) {
        lobbyRoomState->onLobbyState(code, playerCount, spectatorCount);
      }
    });

    networkReceiveSystem->setErrorCallback([this](const std::string &errorMsg) {
      if (lobbyRoomState) {
        lobbyRoomState->onError(errorMsg);
      }
    });

    networkReceiveSystem->setLobbyMessageCallback([this](const std::string &msg, int dur) {
      if (lobbyRoomState) {
        lobbyRoomState->showTemporaryMessage(msg, dur);
      }
    });

    // Player-dead: server told us our player is dead and we should return to menu
    networkReceiveSystem->setPlayerDeadCallback([this](const nlohmann::json &msg) {
      std::string msgType = msg.value("type", "");

      if (msgType == "player_died_spectate") {
        // Player died but game continues - become spectator
        std::cout << "[Game] Player died - switching to spectator mode" << std::endl;

        int aliveCount = msg.value("alive_players", 0);
        std::cout << "[Game] " << aliveCount << " player(s) still alive" << std::endl;

        // Save highscore
        if (msg.contains("score") && menu != nullptr) {
          int finalScore = msg.value("score", 0);
          Difficulty gameDifficulty = menu->getCurrentDifficulty();
          std::string playerName = settings.username;
          HighscoreEntry entry{playerName, finalScore, gameDifficulty};
          menu->getLobbyMenu()->getHighscoreManager().addHighscore(entry);
        }

        if (playingState) {
          std::cout << "[Game] Setting spectator mode to TRUE" << std::endl;
          playingState->setSpectatorMode(true);
          std::cout << "[Game] Spectator mode is now: " << playingState->isSpectator() << std::endl;
        } else {
          std::cerr << "[Game] ERROR: playingState is null, cannot set spectator mode!" << std::endl;
        }

      } else if (msgType == "player_dead") {
        std::cout << "[Game] Game over - returning to menu" << std::endl;

        if (msg.contains("score") && menu != nullptr) {
          int finalScore = msg.value("score", 0);
          Difficulty gameDifficulty = menu->getCurrentDifficulty();
          std::string playerName = settings.username;
          HighscoreEntry entry{playerName, finalScore, gameDifficulty};
          menu->getLobbyMenu()->getHighscoreManager().addHighscore(entry);
        }

        if (m_world) {
          try {
            ecs::ComponentSignature emptySig;
            std::vector<ecs::Entity> allEntities;
            m_world->getEntitiesWithSignature(emptySig, allEntities);
            for (auto e : allEntities) {
              if (m_world->isAlive(e)) {
                m_world->destroyEntity(e);
              }
            }
          } catch (const std::exception &e) {
            std::cerr << "[Game] Error clearing world: " << e.what() << '\n';
          }
        }

        currentState = GameState::MENU;
        if (menu) {
          menu->setState(MenuState::MAIN_MENU);
          menu->refreshHighscoresIfInLobby();
        }
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

    // Save highscore if in solo mode
    if (playingState->isSolo() && lobbyRoomState) {
      int finalScore = playingState->getPlayerScore();
      Difficulty gameDifficulty = lobbyRoomState->getCreationDifficulty();
      std::string playerName = settings.username;

      HighscoreEntry entry{playerName, finalScore, gameDifficulty};
      if (highscoreManager.addHighscore(entry)) {
        std::cout << "[Game] New highscore saved: " << playerName << " - " << finalScore << " points ("
                  << (gameDifficulty == Difficulty::EASY       ? "Easy"
                        : gameDifficulty == Difficulty::MEDIUM ? "Medium"
                                                               : "Expert")
                  << ")" << std::endl;
      }
    }

    // Notify server that we're leaving the lobby/game
    sendLeaveToServer();

    // Tell network receive system to stop accepting snapshots immediately
    if (m_world) {
      if (auto *netRecv = m_world->getSystem<ClientNetworkReceiveSystem>()) {
        netRecv->setAcceptSnapshots(false);
        // Clear lobby-related callbacks to avoid stale UI state after death
        netRecv->setLobbyJoinedCallback(nullptr);
        netRecv->setLobbyStateCallback(nullptr);
        netRecv->setErrorCallback(nullptr);
        netRecv->setLobbyLeftCallback(nullptr);
        netRecv->setPlayerDeadCallback(nullptr);
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

  // Don't update player input if chat input is focused
  if (m_chatUI && m_chatUI->isInputFocused()) {
    return;
  }

  auto &input = m_world->getComponent<ecs::Input>(m_inputEntity);
  input.up = renderer->isKeyPressed(settings.up);
  input.down = renderer->isKeyPressed(settings.down);
  input.left = renderer->isKeyPressed(settings.left);
  input.right = renderer->isKeyPressed(settings.right);
  input.shoot = renderer->isKeyPressed(settings.shoot);
  input.chargedShoot = renderer->isKeyPressed(settings.chargedShoot);
  input.detach = renderer->isKeyPressed(settings.detach);
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
    // Don't process playing state input if chat is focused
    if (playingState && !(m_chatUI && m_chatUI->isInputFocused())) {
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
  if (settings.fullScreen != fullScreen) {
    renderer->setFullscreen(settings.fullScreen);
    fullScreen = settings.fullScreen;
  }

  // Update color blind filter if changed (only if renderer supports it)
  if (settings.colorBlindMode != currentColorBlindMode) {
    auto *colorBlindSupport = dynamic_cast<IColorBlindSupport *>(renderer.get());
    if (colorBlindSupport) {
      colorBlindSupport->setColorBlindMode(settings.colorBlindMode);
      currentColorBlindMode = settings.colorBlindMode;
    }
  }

  if (m_world) {
    m_world->update(deltaTime);
  }

  // Update chat UI
  if (m_chatUI) {
    m_chatUI->update(deltaTime);
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

  // Render chat UI overlay (on top of everything)
  if (m_chatUI) {
    m_chatUI->render();
  }

  // Render end-screen overlay if active (on top of everything else)
  if (m_showEndScreen) {
    renderEndScreen();
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
