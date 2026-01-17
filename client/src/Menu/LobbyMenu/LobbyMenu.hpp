/**
 * @file LobbyMenu.hpp
 * @brief Lobby creation and joining interface
 */

#pragma once
#include "../../../../common/include/Common.hpp"
#include "../../../../common/include/Highscore.hpp"
#include "../../../include/MenuCommon.hpp"
#include "../../../../network/include/INetworkManager.hpp"
#include "../../../include/Settings.hpp"
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Settings;

/**
 * @enum LobbyMenuOption
 * @brief Available lobby menu actions
 */
enum class LobbyMenuOption : std::uint8_t {
  CREATE_LOBBY, ///< Create new lobby
  JOIN_LOBBY, ///< Join existing lobby
  SOLO, ///< Play solo
  CLEAR_HIGHSCORES, ///< Clear highscores
  BACK ///< Return to main menu
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
  LobbyMenu(std::shared_ptr<IRenderer> renderer);
  ~LobbyMenu();

  /**
   * @brief Initialize lobby menu resources
   * @param settings Game settings reference
   */
  void init(Settings &settings);

  /**
   * @brief Render the lobby menu
   * @param windowDims Window dimensions
   */
  void render(const WindowDimensions &windowDims);

  /**
   * @brief Process user input
   * @param currentState Pointer to current menu state
   * @param settings Settings reference for key bindings
   */
  void process(MenuState *currentState, Settings &settings);

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
   * @brief Set solo mode
   * @param solo true to enable solo mode
   */
  void setSolo(bool solo) { m_isSolo = solo; }

  /**
   * @brief Set should enter lobby room flag
   * @param enter true to enter lobby room
   */
  void setShouldEnterLobbyRoom(bool enter) { m_shouldEnterLobbyRoom = enter; }

  /**
   * @brief Set is creating lobby flag
   * @param creating true if creating lobby
   */
  void setIsCreatingLobby(bool creating) { m_isCreatingLobby = creating; }

  /**
   * @brief Set selected difficulty
   * @param diff the difficulty level
   */
  void setSelectedDifficulty(Difficulty diff) { m_selectedDifficulty = diff; }

  /**
   * @brief Start difficulty selection mode
   */
  void startDifficultySelection();

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
   * @brief Refresh highscores from file
   */
  void refreshHighscores();

  /**
   * @brief Check if player is creating a new lobby
   * @return true if creating lobby
   */
  [[nodiscard]] bool isCreatingLobby() const { return m_isCreatingLobby; }

  /**
   * @brief Check if player is playing solo
   * @return true if solo mode
   */
  [[nodiscard]] bool isSolo() const { return m_isSolo; }
  [[nodiscard]] Difficulty getSelectedDifficulty() const { return m_selectedDifficulty; }

private:
  std::shared_ptr<IRenderer> m_renderer;

  void renderBackground(const WindowDimensions &windowDims);
  void renderMenuOptions(const WindowDimensions &windowDims);
  void renderLobbyCodeInput(const WindowDimensions &windowDims);
  void renderDifficultySelection(const WindowDimensions &windowDims);
  void renderHighscores(const WindowDimensions &windowDims);
  void handleMenuNavigation(Settings &settings);
  void handleDifficultyNavigation();
  void handleTextInput();
  void selectCurrentOption(MenuState *currentState);
  void selectDifficultyOption();

  // Settings
  Settings *m_settings = nullptr;

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
  std::vector<std::string> m_menuItems = {"Create Lobby", "Join Lobby", "Solo", "Clear Highscores", "Back"};
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
  bool m_isSolo;

  // Network
  std::shared_ptr<INetworkManager> m_networkManager;
  HighscoreManager m_highscoreManager;

  // Highscore refresh timer
  float m_highscoreRefreshTimer = 0.0F;
  static constexpr float HIGHSCORE_REFRESH_INTERVAL = 3.0F; // Refresh every 3 seconds
};
