/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyRoomState.hpp
*/

#pragma once
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../interface/IRenderer.hpp"
#include "Overlay.hpp"
#include "ParallaxBackground.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace ecs
{
class World;
}

/**
 * @brief Lobby connection state
 */
enum class LobbyConnectionState : std::uint8_t { CONNECTING, JOINED, ERROR };

/**
 * @brief Manages the lobby waiting room before game starts
 */
class LobbyRoomState
{
public:
  LobbyRoomState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world,
                 std::shared_ptr<INetworkManager> networkManager);
  ~LobbyRoomState();

  bool init();
  void update(float deltaTime);
  void render();
  void processInput();
  void cleanup();

  [[nodiscard]] bool shouldStartGame() const { return m_startGameRequested; }
  [[nodiscard]] bool shouldReturnToMenu() const { return m_returnToMenuRequested; }

  // Set lobby mode before requesting
  void setLobbyMode(bool isCreating, const std::string &lobbyCode = "");

  // Send leave message to server
  void sendLeaveLobby();

  // Callbacks from network
  void onLobbyJoined(const std::string &lobbyCode);
  void onLobbyState(const std::string &lobbyCode, int playerCount);
  void onError(const std::string &errorMsg);

  // Send viewport (width/height) to server
  void sendViewportToServer();

private:
  void loadSpriteTextures();
  void freeSpriteTextures();
  void renderLobbyText();
  void requestLobby();

  IRenderer *renderer;
  std::shared_ptr<ecs::World> world;
  std::shared_ptr<INetworkManager> m_networkManager;
  std::unique_ptr<ParallaxBackground> background;
  std::unique_ptr<Overlay> overlay;
  std::unordered_map<std::uint32_t, void *> m_spriteTextures;
  void *m_lobbyFont = nullptr;

  // State tracking
  bool m_startGameRequested = false;
  bool m_returnToMenuRequested = false;
  bool m_lobbyRequested = false;
  float m_timeSinceLobbyRequest = 0.0F;

  // Lobby mode
  bool m_isCreatingLobby = true;
  std::string m_targetLobbyCode;

  // Lobby info
  LobbyConnectionState m_connectionState = LobbyConnectionState::CONNECTING;
  std::string m_lobbyCode;
  int m_playerCount = 0;
  std::string m_errorMessage;
};
