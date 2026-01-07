/**
 * @file Menu.hpp
 * @brief Main menu system with state management
 */

#pragma once
#include "Menu/IntroScreen/IntroScreen.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/ProfileMenu/ProfileMenu.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include "../../common/include/Common.hpp"
#include <string>

class Settings;

/**
 * @class Menu
 * @brief Main menu manager with state machine and rendering
 *
 * Manages all menu screens including main menu, lobby selection,
 * settings, and profile. Handles transitions between states and
 * provides a parallax moon background.
 */
class Menu
{
public:
  /**
   * @brief Construct the menu system
   * @param renderer Pointer to the renderer interface
   * @param settings Reference to game settings
   */
  Menu(IRenderer *renderer, Settings &settings);
  ~Menu();

  /**
   * @brief Initialize menu resources and sub-menus
   */
  void init();

  /**
   * @brief Render current menu state
   */
  void render();

  /**
   * @brief Clean up menu resources
   */
  void cleanup();

  /**
   * @brief Set the current menu state
   * @param newState New menu state to transition to
   */
  void setState(MenuState newState);

  /**
   * @brief Get the current menu state
   * @return Current menu state
   */
  [[nodiscard]] MenuState getState() const;

  /**
   * @brief Check if game should start
   * @return true if player selected to enter game
   */
  [[nodiscard]] bool shouldStartGame() const;

  /**
   * @brief Check if player is creating a new lobby
   * @return true if creating lobby
   */
  [[nodiscard]] bool isCreatingLobby() const;

  /**
   * @brief Get the lobby code player wants to join
   * @return Lobby code string
   */
  [[nodiscard]] std::string getLobbyCodeToJoin() const;

  /**
   * @brief Reset lobby selection flags
   */
  void resetLobbySelection();
  [[nodiscard]] Difficulty getCurrentDifficulty() const;
  [[nodiscard]] LobbyMenu *getLobbyMenu() const { return lobbyMenu; }

  /**
   * @brief Reset highscore refresh flag in lobby menu
   */
  void resetLobbyHighscoresRefresh();

  /**
   * @brief Check if profile menu is currently editing
   * @return true if profile menu is in editing mode
   */
  [[nodiscard]] bool isProfileEditing() const { return profileMenu != nullptr && profileMenu->isEditing(); }

  /**
   * @brief Refresh highscores when entering lobby menu
   */
  void refreshHighscoresIfInLobby();

  /**
   * @brief Process user input for current menu state
   */
  void processInput();

private:
  /**
   * @brief Handle back button navigation
   */
  void processBack();

  /**
   * @brief Draw centered text on screen
   * @param text Text to draw
   * @param yOffset Vertical offset from center
   * @param color Text color
   */
  void drawCenteredText(const std::string &text, int yOffset, const Color &color);

  /**
   * @brief Draw thick bordered rectangle
   * @param posX X position
   * @param posY Y position
   * @param width Rectangle width
   * @param height Rectangle height
   * @param color Border color
   * @param thickness Border thickness
   */
  void drawThickBorderedRect(int posX, int posY, int width, int height, const Color &color, int thickness);

  /**
   * @brief Draw settings menu rectangles
   * @param winWidth Window width
   * @param winHeight Window height
   * @param titleHeight Title section height
   */
  void drawSettingsRectangles(int winWidth, int winHeight, int titleHeight);

  /**
   * @brief Render animated parallax moon background
   * @param winWidth Window width
   * @param winHeight Window height
   * @param renderer Renderer interface
   */
  void renderMoonParalax(int winWidth, int winHeight, IRenderer *renderer);

  IRenderer *renderer;       ///< Renderer interface
  Settings &settings;        ///< Game settings reference
  IntroScreen *introScreen = nullptr;   ///< Intro screen
  MainMenu *mainMenu = nullptr;     ///< Main menu screen
  ProfileMenu *profileMenu = nullptr;   ///< Profile menu screen
  SettingsMenu *settingsMenu = nullptr; ///< Settings menu screen
  LobbyMenu *lobbyMenu = nullptr;       ///< Lobby menu screen

  // Moon Parallax offsets
  float backgroundOffsetX = 0.0f;   ///< Background horizontal offset
  float parallaxOffsetSky = 0.0f;   ///< Sky layer offset
  float parallaxOffsetBack = 0.0f;  ///< Back layer offset
  float parallaxOffsetMid = 0.0f;   ///< Middle layer offset
  float parallaxOffsetFront = 0.0f; ///< Front layer offset
  float parallaxOffsetFloor = 0.0f; ///< Floor layer offset

  void *moonFloor = nullptr;  ///< Floor texture
  void *moonSky = nullptr;    ///< Sky texture
  void *moonMid = nullptr;    ///< Middle layer texture
  void *moonFront = nullptr;  ///< Front layer texture
  void *moonBack = nullptr;   ///< Back layer texture

  void *menu_font = nullptr;
  Difficulty currentDifficulty = Difficulty::MEDIUM;
  MenuState currentState = MenuState::INTRO; ///< Current menu state
};
