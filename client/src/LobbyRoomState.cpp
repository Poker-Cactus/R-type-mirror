/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyRoomState implementation
*/

#include "LobbyRoomState.hpp"
#include "../../engineCore/include/ecs/ComponentSignature.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../interface/KeyCodes.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>

LobbyRoomState::LobbyRoomState(std::shared_ptr<IRenderer> renderer, const std::shared_ptr<ecs::World> &world,
                               std::shared_ptr<INetworkManager> networkManager)
    : renderer(renderer), world(world), m_networkManager(std::move(networkManager)), background(nullptr),
      overlay(nullptr)
{
}

LobbyRoomState::~LobbyRoomState()
{
  cleanup();
}

bool LobbyRoomState::init()
{
  std::cout << "[LobbyRoomState] Initializing..." << '\n';

  if (renderer == nullptr) {
    std::cerr << "LobbyRoomState: Renderer is null" << '\n';
    return false;
  }

  background = std::make_unique<ParallaxBackground>(renderer);
  if (!background->init()) {
    std::cerr << "LobbyRoomState: Failed to initialize parallax background" << '\n';
    return false;
  }

  constexpr std::uint8_t OVERLAY_ALPHA = 180;
  overlay = std::make_unique<Overlay>(renderer, OVERLAY_ALPHA);

  loadSpriteTextures();

  constexpr int WINDOW_WIDTH_THRESHOLD = 1200;
  constexpr int LOBBY_FONT_SIZE_LARGE = 32;
  constexpr int LOBBY_FONT_SIZE_SMALL = 24;
  int winWidth = renderer->getWindowWidth();
  int fontSize = winWidth > WINDOW_WIDTH_THRESHOLD ? LOBBY_FONT_SIZE_LARGE : LOBBY_FONT_SIZE_SMALL;

  try {
    m_lobbyFont = renderer->loadFont("client/assets/font.opf/game.ttf", fontSize);
  } catch (const std::exception &e) {
    std::cerr << "LobbyRoomState: Failed to load font: " << e.what() << '\n';
    m_lobbyFont = nullptr;
  }

  return true;
}

void LobbyRoomState::update([[maybe_unused]] float deltaTime)
{
  if (background) {
    background->update(deltaTime);
  }

  // Only start connection timeout tracking after the request has been sent
  if (m_lobbyRequested && m_connectionState == LobbyConnectionState::CONNECTING) {
    m_timeSinceLobbyRequest += deltaTime;

    // Check for connection timeout
    if (m_timeSinceLobbyRequest >= CONNECTION_TIMEOUT) {
      std::cerr << "[LobbyRoomState] Connection timeout - unable to reach server" << '\n';
      m_connectionState = LobbyConnectionState::ERROR_STATE;
      m_errorMessage = "Unable to connect to server";
    }
  }
  // Update temporary lobby message timer
  if (m_tempMessageTimer > 0.0f) {
    m_tempMessageTimer -= deltaTime;
    if (m_tempMessageTimer <= 0.0f) {
      m_tempMessage.clear();
      m_tempMessageTimer = 0.0f;
    }
  }
}

void LobbyRoomState::render()
{
  if (!world || renderer == nullptr) {
    return;
  }

  if (background) {
    background->render();
  }

  if (overlay) {
    overlay->render();
  }

  if (m_lobbyFont != nullptr) {
    renderLobbyText();
  }
}

void LobbyRoomState::renderLobbyText()
{
  const int winWidth = renderer->getWindowWidth();
  const int winHeight = renderer->getWindowHeight();

  std::string line1;
  std::string line2;
  std::string line3;

  switch (m_connectionState) {
  case LobbyConnectionState::CONNECTING: {
    if (m_lobbyRequested) {
      int elapsedSeconds = static_cast<int>(m_timeSinceLobbyRequest);
      int remainingSeconds = static_cast<int>(CONNECTION_TIMEOUT - m_timeSinceLobbyRequest);
      line1 = "Connecting to lobby" + std::string(m_joinAsSpectator ? " (Spectator)" : "") + "... (" +
        std::to_string(elapsedSeconds) + "s)";
      line2 = "Timeout in " + std::to_string(std::max(0, remainingSeconds)) + "s";
    } else {
      // If we're not yet requested, show a simple preparing message (we auto-request on enter)
      line1 = "Preparing lobby...";
      line2 = "Please wait";
    }
    break;
  }
  case LobbyConnectionState::JOINED:
    if (m_spectatorCount > 0) {
      line1 = "Lobby: " + m_lobbyCode + " (" + std::to_string(m_playerCount) + " player" +
        (m_playerCount != 1 ? "s" : "") + ", " + std::to_string(m_spectatorCount) + " ghost" +
        (m_spectatorCount != 1 ? "s" : "") + ")";
    } else {
      line1 = "Lobby: " + m_lobbyCode + " (" + std::to_string(m_playerCount) + " player" +
        (m_playerCount != 1 ? "s" : "") + ")";
    }
    line2 = "Press X to start, BACKSPACE to leave";
    // Extra hint: allow players to opt-in as ghost (spectator) from the waiting lobby
    line3 = "Press G to be a ghost";
    break;
  case LobbyConnectionState::ERROR_STATE:
    line1 = "Error: " + m_errorMessage;
    line2 = "Press BACKSPACE to return to menu";
    break;
  }

  constexpr std::uint8_t TEXT_WHITE = 255;
  constexpr std::uint8_t TEXT_ALPHA = 255;
  const Color textColor = {.r = TEXT_WHITE, .g = TEXT_WHITE, .b = TEXT_WHITE, .a = TEXT_ALPHA};

  int textWidth1 = 0;
  int textHeight1 = 0;
  renderer->getTextSize(m_lobbyFont, line1, textWidth1, textHeight1);

  const int textPosX1 = (winWidth - textWidth1) / 2;
  const int textPosY1 = ((winHeight - textHeight1) / 2) - (line2.empty() ? 0 : 20);

  renderer->drawText(m_lobbyFont, line1, textPosX1, textPosY1, textColor);

  if (!line2.empty()) {
    int textWidth2 = 0;
    int textHeight2 = 0;
    renderer->getTextSize(m_lobbyFont, line2, textWidth2, textHeight2);

    constexpr int LINE_SPACING = 10;
    const int textPosX2 = (winWidth - textWidth2) / 2;
    const int textPosY2 = textPosY1 + textHeight1 + LINE_SPACING;

    renderer->drawText(m_lobbyFont, line2, textPosX2, textPosY2, textColor);
    // Optional third line (additional hint)
    if (!line3.empty()) {
      int textWidth3 = 0;
      int textHeight3 = 0;
      renderer->getTextSize(m_lobbyFont, line3, textWidth3, textHeight3);
      const int textPosX3 = (winWidth - textWidth3) / 2;
      const int textPosY3 = textPosY2 + textHeight2 + LINE_SPACING;
      renderer->drawText(m_lobbyFont, line3, textPosX3, textPosY3, textColor);
      // If the local client requested spectator mode, show a confirmatory blue text below
      if (m_joinAsSpectator) {
        const Color blueColor = {.r = 100, .g = 149, .b = 237, .a = 255}; // cornflower blue
        std::string ghostActive = "Ghost mode Active";
        int textWidth4 = 0;
        int textHeight4 = 0;
        renderer->getTextSize(m_lobbyFont, ghostActive, textWidth4, textHeight4);
        const int textPosX4 = (winWidth - textWidth4) / 2;
        const int textPosY4 = textPosY3 + textHeight3 + LINE_SPACING;
        renderer->drawText(m_lobbyFont, ghostActive, textPosX4, textPosY4, blueColor);
      }
    }
  }

  // Draw temporary lobby-wide message (centered, red)
  if (!m_tempMessage.empty() && m_lobbyFont != nullptr) {
    const Color red = {.r = 255, .g = 0, .b = 0, .a = 255};
    int tmw = 0, tmh = 0;
    renderer->getTextSize(m_lobbyFont, m_tempMessage, tmw, tmh);
    const int tmx = (winWidth - tmw) / 2;
    const int tmy = winHeight / 4; // place above center
    renderer->drawText(m_lobbyFont, m_tempMessage, tmx, tmy, red);
  }
}

void LobbyRoomState::processInput()
{
  if (renderer == nullptr || m_networkManager == nullptr) {
    std::cerr << "[LobbyRoomState] renderer or networkManager is null!" << '\n';
    return;
  }

  if (m_startGameRequested || m_returnToMenuRequested) {
    return;
  }

  // BACKSPACE to return to menu (send leave message first)
  if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
    std::cout << "[LobbyRoomState] BACKSPACE pressed - returning to menu" << '\n';
    sendLeaveLobby();
    m_returnToMenuRequested = true;
    return;
  }

  // G to toggle ghost/spectator mode while already in lobby (JOINED)
  if (m_connectionState == LobbyConnectionState::JOINED && renderer->isKeyJustPressed(KeyCode::KEY_G) &&
      !m_lobbyRequested) {
    // Toggle local requested spectator flag for UI
    m_joinAsSpectator = !m_joinAsSpectator;
    std::cout << "[LobbyRoomState] Sending toggle_spectator (now " << (m_joinAsSpectator ? "SPECTATOR" : "PLAYER")
              << ") to server" << '\n';

    if (m_networkManager != nullptr) {
      nlohmann::json message;
      message["type"] = "toggle_spectator";
      message["spectator"] = m_joinAsSpectator;
      std::string serialized = message.dump();
      const auto capnpSerialized = m_networkManager->getPacketHandler()->serialize(serialized);
      m_networkManager->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(capnpSerialized.data()), capnpSerialized.size()),
        0);
      // We don't leave the lobby; wait for server lobby_state update
    }
  }

  // ENTER to confirm and send lobby request
  if (m_connectionState == LobbyConnectionState::CONNECTING && !m_lobbyRequested &&
      renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    std::cout << "[LobbyRoomState] Sending lobby request (Spectator: " << (m_joinAsSpectator ? "YES" : "NO") << ")"
              << '\n';
    requestLobby();
    m_lobbyRequested = true;
    m_timeSinceLobbyRequest = 0.0F;
  }

  // Only allow starting game if we're in a lobby
  if (m_connectionState == LobbyConnectionState::JOINED && renderer->isKeyJustPressed(KeyCode::KEY_X)) {
    std::cout << "[LobbyRoomState] Sending start game request to server" << '\n';
    // Ensure server knows our current viewport before starting the game
    sendViewportToServer();

    nlohmann::json message;
    message["type"] = "start_game";

    std::string serialized = message.dump();

    // Serialize with Cap'n Proto like the viewport message
    const auto capnpSerialized = m_networkManager->getPacketHandler()->serialize(serialized);

    m_networkManager->send(
      std::span<const std::byte>(reinterpret_cast<const std::byte *>(capnpSerialized.data()), capnpSerialized.size()),
      0);
    m_startGameRequested = true;
  }
}

void LobbyRoomState::setLobbyMode(bool isCreating, const std::string &lobbyCode, Difficulty difficulty, bool isSolo,
                                  AIDifficulty aiDifficulty, GameMode mode)
{
  m_isCreatingLobby = isCreating;
  m_isSolo = isSolo;
  m_targetLobbyCode = lobbyCode;
  m_creationDifficulty = difficulty;
  m_aiDifficulty = aiDifficulty;
  m_gameMode = mode;
  m_lobbyRequested = false; // Reset so we can request again
  m_connectionState = LobbyConnectionState::CONNECTING;
  m_returnToMenuRequested = false;
  m_joinAsSpectator = false; // Reset spectator mode
  std::cout << "[LobbyRoomState] Mode set: " << (isCreating ? "CREATE" : "JOIN")
            << (lobbyCode.empty() ? "" : " code=" + lobbyCode) << (isSolo ? " SOLO" : "")
            << " AI Difficulty: " << static_cast<int>(aiDifficulty)
            << " Game Mode: " << (mode == GameMode::CLASSIC ? "CLASSIC" : "ENDLESS") << '\n';

  // Immediately request the lobby so we go straight into the lobby flow
  // This removes the intermediate pre-join page that showed "Spectator mode: OFF (Press G)".
  try {
    requestLobby();
    m_lobbyRequested = true;
    m_timeSinceLobbyRequest = 0.0F;
    std::cout << "[LobbyRoomState] Auto-requesting lobby on setLobbyMode" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[LobbyRoomState] Failed to auto-request lobby: " << e.what() << std::endl;
  }
}

void LobbyRoomState::sendLeaveLobby()
{
  if (m_networkManager == nullptr) {
    return;
  }

  std::cout << "[LobbyRoomState] Sending leave_lobby message to server" << '\n';

  nlohmann::json message;
  message["type"] = "leave_lobby";

  std::string serialized = message.dump();
  const auto capnpSerialized = m_networkManager->getPacketHandler()->serialize(serialized);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(capnpSerialized.data()), capnpSerialized.size()), 0);
}

void LobbyRoomState::requestLobby()
{
  std::cout << "[LobbyRoomState] Requesting lobby from server" << '\n';

  nlohmann::json message;

  if (m_isCreatingLobby) {
    message["type"] = "request_lobby";
    message["action"] = "create";
    message["difficulty"] = static_cast<int>(m_creationDifficulty);
    message["ai_difficulty"] = static_cast<int>(m_aiDifficulty);
    message["mode"] = static_cast<int>(m_gameMode);
    message["spectator"] = m_joinAsSpectator;
    if (m_isSolo) {
      message["solo"] = true;
    }
    std::cout << "[LobbyRoomState] Creating lobby with AI difficulty: " << static_cast<int>(m_aiDifficulty) << '\n';
  } else {
    message["type"] = "request_lobby";
    message["action"] = "join";
    message["lobby_code"] = m_targetLobbyCode;
    message["spectator"] = m_joinAsSpectator;
  }

  // Include local username if available
  if (m_settings != nullptr && !m_settings->username.empty()) {
    message["username"] = m_settings->username;
  }
  std::string serialized = message.dump();
  std::cout << "[LobbyRoomState] Sending message: " << serialized << '\n';

  // Serialize with Cap'n Proto like the viewport message
  const auto capnpSerialized = m_networkManager->getPacketHandler()->serialize(serialized);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(capnpSerialized.data()), capnpSerialized.size()), 0);
  std::cout << "[LobbyRoomState] Message sent to network manager" << '\n';
}

void LobbyRoomState::setSettings(Settings *settings)
{
  m_settings = settings;
}

void LobbyRoomState::cleanup()
{
  freeSpriteTextures();
  if (renderer != nullptr && m_lobbyFont != nullptr)
    renderer->freeFont(m_lobbyFont);
  m_lobbyFont = nullptr;
}

void LobbyRoomState::sendViewportToServer()
{
  if (m_networkManager == nullptr || renderer == nullptr) {
    return;
  }

  nlohmann::json viewport;
  viewport["type"] = "viewport";
  viewport["width"] = static_cast<std::uint32_t>(renderer->getWindowWidth());
  viewport["height"] = static_cast<std::uint32_t>(renderer->getWindowHeight());

  std::string serialized = viewport.dump();
  const auto capnpSerialized = m_networkManager->getPacketHandler()->serialize(serialized);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(capnpSerialized.data()), capnpSerialized.size()), 0);

  std::cout << "[LobbyRoomState] Sent viewport " << viewport["width"] << "x" << viewport["height"] << '\n';
}

void LobbyRoomState::loadSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  try {
    void *playerTexture = renderer->loadTexture("client/assets/r-typesheet1.gif");
    m_spriteTextures[static_cast<std::uint32_t>(ecs::SpriteId::PLAYER_SHIP)] = playerTexture;
  } catch (const std::exception &e) {
    std::cerr << "LobbyRoomState: Failed to load player texture: " << e.what() << '\n';
  }
}

void LobbyRoomState::freeSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  for (auto &[spriteId, texture] : m_spriteTextures) {
    if (texture != nullptr) {
      renderer->freeTexture(texture);
    }
  }
  m_spriteTextures.clear();
}

// ============================================================================
// Network Callbacks
// ============================================================================

void LobbyRoomState::onLobbyJoined(const std::string &lobbyCode)
{
  std::cout << "[LobbyRoomState] Successfully joined lobby: " << lobbyCode << '\n';
  // Mark joined and reset request/timeout state so UI becomes responsive
  m_connectionState = LobbyConnectionState::JOINED;
  m_lobbyCode = lobbyCode;
  m_lobbyRequested = false;
  m_timeSinceLobbyRequest = 0.0F;

  // Immediately send our viewport to the server so it has correct dimensions before the game begins.
  sendViewportToServer();
}

void LobbyRoomState::showTemporaryMessage(const std::string &message, int durationSeconds)
{
  // Show message and ensure lobby input is active again (don't block keys)
  m_tempMessage = message;
  m_tempMessageTimer = static_cast<float>(std::max(1, durationSeconds));
  // Clear transient request flags so user can interact while the message is shown
  m_startGameRequested = false;
  m_returnToMenuRequested = false;
  m_lobbyRequested = false;
}

void LobbyRoomState::onLobbyState(const std::string &lobbyCode, int playerCount, int spectatorCount)
{
  std::cout << "[LobbyRoomState] Lobby " << lobbyCode << " state update: " << playerCount << " players";
  std::cout << " and " << spectatorCount << " spectators" << std::endl;
  m_lobbyCode = lobbyCode;
  // Update counts and ensure we reflect that we're joined
  m_playerCount = playerCount;
  m_spectatorCount = spectatorCount;
  m_connectionState = LobbyConnectionState::JOINED;
  m_lobbyRequested = false;
  m_timeSinceLobbyRequest = 0.0F;
}

void LobbyRoomState::onError(const std::string &errorMsg)
{
  std::cerr << "[LobbyRoomState] Error: " << errorMsg << '\n';
  m_connectionState = LobbyConnectionState::ERROR_STATE;
  m_errorMessage = errorMsg;
}
