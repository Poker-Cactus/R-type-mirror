#pragma once
#include "Menu/LoadingMenu/LoadingMenu.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/ProfileMenu/ProfileMenu.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include "../../common/include/Common.hpp"
#include <string>

class Menu
{
public:
  Menu(IRenderer *renderer);
  ~Menu();

  void init();
  void render();
  void cleanup();

  void setState(MenuState newState);
  [[nodiscard]] MenuState getState() const;
  [[nodiscard]] bool shouldStartGame() const;

  // Lobby selection info
  [[nodiscard]] bool isCreatingLobby() const;
  [[nodiscard]] std::string getLobbyCodeToJoin() const;
  void resetLobbySelection();
  [[nodiscard]] Difficulty getCurrentDifficulty() const { return currentDifficulty; }
  [[nodiscard]] LobbyMenu *getLobbyMenu() const { return lobbyMenu; }

  void processInput();

private:
  void processBack();

  void drawCenteredText(const std::string &text, int yOffset, const Color &color);
  void drawThickBorderedRect(int posX, int posY, int width, int height, const Color &color, int thickness);
  void drawSettingsRectangles(int winWidth, int winHeight, int titleHeight);
  void renderMoonParalax(int winWidth, int winHeight, IRenderer *renderer);

  IRenderer *renderer;
  MainMenu *mainMenu = nullptr;
  LoadingMenu *loadingMenu = nullptr;
  ProfileMenu *profileMenu = nullptr;
  SettingsMenu *settingsMenu = nullptr;
  LobbyMenu *lobbyMenu = nullptr;

  // Moon Paralax
  float backgroundOffsetX = 0.0f;
  float parallaxOffsetSky = 0.0f;
  float parallaxOffsetBack = 0.0f;
  float parallaxOffsetMid = 0.0f;
  float parallaxOffsetFront = 0.0f;
  float parallaxOffsetFloor = 0.0f;

  void *moonFloor = nullptr;
  void *moonSky = nullptr;
  void *moonMid = nullptr;
  void *moonFront = nullptr;
  void *moonBack = nullptr;

  void *menu_font = nullptr;
  LoadingScreen *loadingScreen = nullptr;
  MenuState currentState = MenuState::LOADING;
  Difficulty currentDifficulty = Difficulty::MEDIUM;
};
