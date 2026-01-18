/**
 * @file LobbyRoomState.hpp
 * @brief Pre-game lobby waiting room state
 */

#pragma once
#include "../../common/include/Common.hpp"
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../include/Settings.hpp"
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
 * @enum LobbyConnectionState
 * @brief Lobby connection status
 */
enum class LobbyConnectionState : std::uint8_t {
  CONNECTING, ///< Attempting to connect
  JOINED, ///< Successfully joined
  ERROR_STATE ///< Connection error
};

/**
 * @class LobbyRoomState
 * @brief Manages the pre-game lobby waiting room
 *
 * Players wait in this state after creating or joining a lobby
 * until the game starts. Shows lobby code, player count, and
 * provides ability to leave.
 */
class LobbyRoomState
{
public:
  static constexpr float CONNECTION_TIMEOUT = 5.0f; ///< Connection timeout in seconds

  /**
   * @brief Construct the lobby room state
   * @param renderer Renderer interface
   * @param world Shared pointer to ECS world
   * @param networkManager Network manager for lobby communication
   */
  LobbyRoomState(std::shared_ptr<IRenderer> renderer, const std::shared_ptr<ecs::World> &world,
                 std::shared_ptr<INetworkManager> networkManager);
  ~LobbyRoomState();

  /**
   * @brief Initialize lobby room resources
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Update lobby state
   * @param deltaTime Time elapsed since last update
   */
  void update(float deltaTime);

  /**
   * @brief Render lobby room UI
   */
  void render();

  /**
   * @brief Process user input in lobby
   */
  void processInput();

  /**
   * @brief Clean up lobby room resources
   */
  void cleanup();

  /**
   * @brief Check if game should start
   * @return true if server signaled game start
   */
  [[nodiscard]] bool shouldStartGame() const { return m_startGameRequested; }

  /**
   * @brief Check if player wants to return to menu
   * @return true if returning to menu
   */
  [[nodiscard]] bool shouldReturnToMenu() const { return m_returnToMenuRequested; }

  /**
   * @brief Check if this is solo mode
   * @return true if solo mode
   */
  [[nodiscard]] bool isSolo() const { return m_isSolo; }

  /**
   * @brief Get the creation difficulty
   * @return the difficulty
   */
  [[nodiscard]] Difficulty getCreationDifficulty() const { return m_creationDifficulty; }

  /**
   * @brief Set lobby mode before connection
   * @param isCreating Whether creating new lobby
   * @param lobbyCode Lobby code (for joining)
   * @param difficulty Game difficulty level
   * @param aiDifficulty AI difficulty level
   * @param isSolo Whether this is a solo game
   */
  void setLobbyMode(bool isCreating, const std::string &lobbyCode = "", Difficulty difficulty = Difficulty::MEDIUM,
                    bool isSolo = false, AIDifficulty aiDifficulty = AIDifficulty::MEDIUM,
                    GameMode mode = GameMode::CLASSIC);

  /**
   * @brief Send leave lobby message to server
   */
  void sendLeaveLobby();

  /**
   * @brief Network callback: lobby joined successfully
   * @param lobbyCode Assigned lobby code
   */
  void onLobbyJoined(const std::string &lobbyCode);

  /**
   * @brief Network callback: lobby state update
    * @param lobbyCode Current lobby code
    * @param playerCount Number of players in lobby
    * @param spectatorCount Number of spectators in lobby
   */
    void onLobbyState(const std::string &lobbyCode, int playerCount, int spectatorCount);

  /**
   * @brief Network callback: error occurred
   * @param errorMsg Error message
   */
  void onError(const std::string &errorMsg);

  /**
   * @brief Show a temporary lobby-wide message for the given duration (seconds)
   */
  void showTemporaryMessage(const std::string &message, int durationSeconds);

  /**
   * @brief Send viewport dimensions to server
   */
  void sendViewportToServer();
  /**
   * @brief Provide Settings pointer so this state can include username in lobby requests
   */
  void setSettings(Settings *settings);

private:
  void loadSpriteTextures();
  void freeSpriteTextures();
  void renderLobbyText();
  void requestLobby();

  std::shared_ptr<IRenderer> renderer;
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
  Difficulty m_creationDifficulty = Difficulty::MEDIUM;
  AIDifficulty m_aiDifficulty = AIDifficulty::MEDIUM;
  GameMode m_gameMode = GameMode::CLASSIC;

  // Lobby mode
  bool m_isCreatingLobby = true;
  bool m_isSolo = false;
  std::string m_targetLobbyCode;
  bool m_joinAsSpectator = false;

  // Lobby info
  LobbyConnectionState m_connectionState = LobbyConnectionState::CONNECTING;
  std::string m_lobbyCode;
  int m_playerCount = 0;
  int m_spectatorCount = 0;
  std::string m_errorMessage;
  // Local settings pointer to include username in requests
  Settings *m_settings = nullptr;
  // Temporary lobby-wide message display
  std::string m_tempMessage;
  float m_tempMessageTimer = 0.0f;

};
