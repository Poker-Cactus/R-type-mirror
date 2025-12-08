#pragma once
#include "../interface/IRenderer.hpp"
#include <array>
#include <string>

enum class MenuState { LOADING, MAIN_MENU, LOBBY, SETTINGS, PROFILE };

class Menu
{
  public:
    Menu(IRenderer *renderer);
    ~Menu();

    void init();
    void render();
    void cleanup();

    void setState(MenuState newState);
    MenuState getState() const;

    void processInput();

  private:
    void renderLoading(int winWidth, int winHeight);
    void renderMainMenu(int winWidth, int winHeight);
    void renderProfile(int winWidth, int winHeight);
    void renderLobby(int winWidth, int winHeight);
    void renderSettings(int winWidth, int winHeight);

    void processLoading();
    void processMainMenu();
    void processProfile();
    void processLobby();
    void processSettings();
    void processBack();

    // Helper pour afficher du texte centré
    void drawCenteredText(const std::string &text, int yOffset, const Color &color);

    IRenderer *renderer;
    void *logo = nullptr;
    void *backgroundTexture = nullptr;
    void *menu_font = nullptr;
    float blinkTimer = 0.0f;
    MenuState currentState = MenuState::LOADING;

    // Textes du menu principal
    std::array<std::string, 4> mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
    int currentMenuIndex = 0;
};