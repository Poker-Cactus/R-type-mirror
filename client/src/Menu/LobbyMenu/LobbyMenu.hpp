/**
 * @file LobbyMenu.hpp
 * @brief Lobby creation and joining interface
 */

#pragma once
#include "../../../../network/include/INetworkManager.hpp"
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

/**
 * @struct WindowDimensions
 * @brief Window size container to prevent parameter confusion
 */
struct WindowDimensions {
  int width;  ///< Window width in pixels
  int height; ///< Window height in pixels
};

/**
 * @enum LobbyMenuOption
 * @brief Available lobby menu actions
 */
enum class LobbyMenuOption : std::uint8_t {
  CREATE_LOBBY, ///< Create new lobby
  JOIN_LOBBY,   ///< Join existing lobby
  BACK          ///< Return to main menu
};

/**
 * @class LobbyMenu
 * @brief Interface for creating and joining game lobbies
 *
 * Allows players to either create a new lobby or join an existing one
 * by entering a lobby code. Integrates with the network manager.
 */
class LobbyMenu
{
public:
  LobbyMenu() = default;
  ~LobbyMenu();

  /**
   * @brief Initialize lobby menu resources
   * @param renderer Renderer interface
   */
  void init(IRenderer *renderer);

  /**
   * @brief Render the lobby menu
   * @param windowDims Window dimensions
   * @param renderer Renderer interface
   */
  void render(const WindowDimensions &windowDims, IRenderer *renderer);

  /**
   * @brief Process user input
   * @param renderer Renderer interface
   * @param currentState Pointer to current menu state
   */
  void process(IRenderer *renderer, MenuState *currentState);

  /**
   * @brief Clean up lobby menu resources
   */
  void cleanup();

  /**
   * @brief Set network manager for lobby operations
   * @param networkManager Network manager instance
   */
  void setNetworkManager(std::shared_ptr<INetworkManager> networkManager);

  /**
   * @brief Check if player wants to enter lobby room
   * @return true if transition to lobby room requested
   */
  [[nodiscard]] bool shouldEnterLobbyRoom() const { return m_shouldEnterLobbyRoom; }

  /**
   * @brief Reset lobby room transition flag
   */
  void resetLobbyRoomFlag() { m_shouldEnterLobbyRoom = false; }

  /**
   * @brief Get the lobby code to join
   * @return Lobby code string
   */
  [[nodiscard]] const std::string &getLobbyCodeToJoin() const { return m_lobbyCodeInput; }

  /**
   * @brief Check if player is creating a new lobby
   * @return true if creating lobby
   */
  [[nodiscard]] bool isCreatingLobby() const { return m_isCreatingLobby; }

private:
  void renderBackground(const WindowDimensions &windowDims, IRenderer *renderer);
  void renderMenuOptions(const WindowDimensions &windowDims, IRenderer *renderer);
  void renderLobbyCodeInput(const WindowDimensions &windowDims, IRenderer *renderer);
  void handleMenuNavigation(IRenderer *renderer);
  void handleTextInput(IRenderer *renderer);
  void selectCurrentOption(MenuState *currentState);

  // Assets
  void *m_font = nullptr;
  void *m_titleFont = nullptr;
  void *m_moonSky = nullptr;
  void *m_moonBack = nullptr;
  void *m_moonMid = nullptr;
  void *m_moonFront = nullptr;
  void *m_moonFloor = nullptr;

  // Parallax offsets
  float m_parallaxOffsetSky = 0.0F;
  float m_parallaxOffsetBack = 0.0F;
  float m_parallaxOffsetMid = 0.0F;
  float m_parallaxOffsetFront = 0.0F;
  float m_parallaxOffsetFloor = 0.0F;

  // Menu state
  std::vector<std::string> m_menuItems = {"Create Lobby", "Join Lobby", "Back"};
  std::size_t m_currentIndex = 0;
  bool m_isEnteringCode = false;
  std::string m_lobbyCodeInput;
  static constexpr std::size_t MAX_LOBBY_CODE_LENGTH = 10;

  // Transition flags
  bool m_shouldEnterLobbyRoom = false;
  bool m_isCreatingLobby = false;

  // Network
  std::shared_ptr<INetworkManager> m_networkManager;
};
