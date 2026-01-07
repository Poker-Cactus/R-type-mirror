/**
 * @file LobbyMenu.hpp
 * @brief Lobby creation and joining interface
 */

#pragma once
#include "../../../../common/include/Common.hpp"
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
  LobbyMenu(std::shared_ptr <IRenderer> renderer);
  ~LobbyMenu();

  /**
   * @brief Initialize lobby menu resources
   * @param renderer Renderer interface
   */
  void init();

  /**
   * @brief Render the lobby menu
   * @param windowDims Window dimensions
   * @param renderer Renderer interface
   */
  void render(const WindowDimensions &windowDims);

  /**
   * @brief Process user input
   * @param renderer Renderer interface
   * @param currentState Pointer to current menu state
   */
  void process(MenuState *currentState);

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
  [[nodiscard]] Difficulty getSelectedDifficulty() const { return m_selectedDifficulty; }

private:
  std::shared_ptr<IRenderer> m_renderer;

  void renderBackground(const WindowDimensions &windowDims);
  void renderMenuOptions(const WindowDimensions &windowDims);
  void renderLobbyCodeInput(const WindowDimensions &windowDims);
  void renderDifficultySelection(const WindowDimensions &windowDims);
  void handleMenuNavigation();
  void handleDifficultyNavigation();
  void handleTextInput();
  void selectCurrentOption(MenuState *currentState);
  void selectDifficultyOption();

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
  
  // Difficulty selection
  bool m_isSelectingDifficulty = false;
  std::vector<std::string> m_difficultyItems = {"Easy", "Medium", "Expert"};
  std::size_t m_difficultyIndex = 1; // Default to Medium
  Difficulty m_selectedDifficulty = Difficulty::MEDIUM;

  // Transition flags
  bool m_shouldEnterLobbyRoom;
  bool m_isCreatingLobby;

  // Network
  std::shared_ptr<INetworkManager> m_networkManager;
};
