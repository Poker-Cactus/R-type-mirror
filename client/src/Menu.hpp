#pragma once
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/LoadingMenu/LoadingMenu.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
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

    void processInput();

  private:
    void processBack();

    void drawCenteredText(const std::string &text, int yOffset, const Color &color);
    void drawThickBorderedRect(int x, int y, int width, int height, const Color &color, int thickness);
    void drawSettingsRectangles(int winWidth, int winHeight, int titleHeight);
    void renderMoonParalax(IRenderer *renderer, int winWidth, int winHeight);

    // Moon paralax
    void *moonFloor = nullptr;
    void *moonSky = nullptr;
    void *moonMid = nullptr;
    void *moonFront = nullptr;
    void *moonBack = nullptr;

    float backgroundOffsetX = 0.0f;
    float parallaxOffsetSky = 0.0f;
    float parallaxOffsetBack = 0.0f;
    float parallaxOffsetMid = 0.0f;
    float parallaxOffsetFront = 0.0f;
    float parallaxOffsetFloor = 0.0f;

    IRenderer *renderer;
    MainMenu *mainMenu;
    LoadingMenu *loadingMenu;
    ProfileMenu *profileMenu;
    SettingsMenu *settingsMenu;

    void *menu_font = nullptr;
    LoadingScreen *loadingScreen = nullptr;
    LobbyMenu *lobbyMenu;
    MenuState currentState = MenuState::LOADING;
};
