/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyMenu - Menu for creating and joining game lobbies
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
 * @brief Window dimensions structure to prevent accidental parameter swaps
 */
struct WindowDimensions {
  int width;
  int height;
};

/**
 * @brief Lobby selection options
 */
enum class LobbyMenuOption : std::uint8_t { CREATE_LOBBY, JOIN_LOBBY, BACK };

/**
 * @brief Menu for creating and joining game lobbies
 */
class LobbyMenu
{
public:
  LobbyMenu(std::shared_ptr <IRenderer> renderer);
  ~LobbyMenu();

  void init();
  void render(const WindowDimensions &windowDims);
  void process(MenuState *currentState);
  void cleanup();

  // Network integration
  void setNetworkManager(std::shared_ptr<INetworkManager> networkManager);

  // Check if user wants to proceed to lobby room
  [[nodiscard]] bool shouldEnterLobbyRoom() const { return m_shouldEnterLobbyRoom; }
  void resetLobbyRoomFlag() { m_shouldEnterLobbyRoom = false; }

  // Get the lobby code to join (if joining existing)
  [[nodiscard]] const std::string &getLobbyCodeToJoin() const { return m_lobbyCodeInput; }
  [[nodiscard]] bool isCreatingLobby() const { return m_isCreatingLobby; }

private:
  std::shared_ptr<IRenderer> m_renderer;

  void renderBackground(const WindowDimensions &windowDims);
  void renderMenuOptions(const WindowDimensions &windowDims);
  void renderLobbyCodeInput(const WindowDimensions &windowDims);
  void handleMenuNavigation();
  void handleTextInput();
  void selectCurrentOption(MenuState *currentState);

  // Assets
  void *m_font;
  void *m_titleFont;
  void *m_moonSky;
  void *m_moonBack;
  void *m_moonMid;
  void *m_moonFront;
  void *m_moonFloor;

  // Parallax offsets
  float m_parallaxOffsetSky;
  float m_parallaxOffsetBack;
  float m_parallaxOffsetMid;
  float m_parallaxOffsetFront;
  float m_parallaxOffsetFloor;

  // Menu state
  std::vector<std::string> m_menuItems = {"Create Lobby", "Join Lobby", "Back"};
  std::size_t m_currentIndex;
  bool m_isEnteringCode;
  std::string m_lobbyCodeInput;
  static constexpr std::size_t MAX_LOBBY_CODE_LENGTH = 10;

  // Transition flags
  bool m_shouldEnterLobbyRoom;
  bool m_isCreatingLobby;

  // Network
  std::shared_ptr<INetworkManager> m_networkManager;
};
