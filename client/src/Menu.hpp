#pragma once
#include "../interface/IRenderer.hpp"
#include <array>
#include <string>

enum class MenuState { LOADING, MAIN_MENU, LOBBY, SETTINGS, PROFILE, EXIT };

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
    class LoadingScreen
    {
      public:
        LoadingScreen(IRenderer *renderer, void *font);
        ~LoadingScreen() = default;

        void start();
        void stop();
        bool update(int winWidth, int winHeight);
        bool isActive() const;
        void setDuration(float duration);

      private:
        IRenderer *renderer;
        void *font;
        bool active = false;
        float timer = 0.0f;
        float duration = 2.0f;
        float rotation = 0.0f;
    };
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

    void drawCenteredText(const std::string &text, int yOffset, const Color &color);
    void drawThickBorderedRect(int x, int y, int width, int height, const Color &color, int thickness);
    void drawSettingsRectangles(int winWidth, int winHeight, int titleHeight);

    IRenderer *renderer;
    void *logo = nullptr;
    void *backgroundTexture = nullptr;
    void *menu_font = nullptr;
    void *title_font = nullptr;
    float blinkTimer = 0.0f;
    float backgroundOffsetX = 0.0f;
    float parallaxOffsetSky = 0.0f;
    float parallaxOffsetBack = 0.0f;
    float parallaxOffsetMid = 0.0f;
    float parallaxOffsetFront = 0.0f;
    float parallaxOffsetFloor = 0.0f;
    void *planet = nullptr;
    bool isZooming = false;
    float zoomTimer = 0.0f;
    float zoomScale = 0.3f;
    float zoomDuration = 0.8f;
    LoadingScreen *loadingScreen = nullptr;
    MenuState currentState = MenuState::LOADING;

    void *moonFloor = nullptr;
    void *moonSky = nullptr;
    void *moonMid = nullptr;
    void *moonFront = nullptr;
    void *moonBack = nullptr;
    // Textes du menu principal
    std::array<std::string, 4> mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
    int currentMenuIndex = 0;
};