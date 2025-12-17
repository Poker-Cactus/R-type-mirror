#pragma once
#include "Menu/LoadingMenu/LoadingMenu.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/ProfileMenu/ProfileMenu.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
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

  void processInput();

private:
  void processBack();

  void drawCenteredText(const std::string &text, int yOffset, const Color &color);
  void drawThickBorderedRect(int posX, int posY, int width, int height, const Color &color, int thickness);
  void drawSettingsRectangles(int winWidth, int winHeight, int titleHeight);

  IRenderer *renderer;
  MainMenu *mainMenu = nullptr;
  LoadingMenu *loadingMenu = nullptr;
  ProfileMenu *profileMenu = nullptr;
  SettingsMenu *settingsMenu = nullptr;
  LobbyMenu *lobbyMenu = nullptr;

  void *menu_font = nullptr;
  LoadingScreen *loadingScreen = nullptr;
  MenuState currentState = MenuState::LOADING;
};
