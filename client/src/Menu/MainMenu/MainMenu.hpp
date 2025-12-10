#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <string>

class MainMenu
{
  public:
    MainMenu() = default;
    ~MainMenu() {};
    void init(IRenderer *renderer);
    void render(int winWidth, int winHeight, IRenderer *renderer);
    void process(MenuState *currentState, IRenderer *renderer);

  private:
    void *font;
    float backgroundOffsetX = 0.0f;
    float parallaxOffsetSky = 0.0f;
    float parallaxOffsetBack = 0.0f;
    float parallaxOffsetMid = 0.0f;
    float parallaxOffsetFront = 0.0f;
    float parallaxOffsetFloor = 0.0f;
    void *planet = nullptr;

    void *moonFloor = nullptr;
    void *moonSky = nullptr;
    void *moonMid = nullptr;
    void *moonFront = nullptr;
    void *moonBack = nullptr;

    std::array<std::string, 4> mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
    int currentMenuIndex = 0;
};