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
  LobbyMenu() = default;
  ~LobbyMenu();

  void init(IRenderer *renderer);
  void render(const WindowDimensions &windowDims, IRenderer *renderer);
  void process(IRenderer *renderer, MenuState *currentState);
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
