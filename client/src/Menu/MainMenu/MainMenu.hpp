#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <string>

class MainMenu
{
public:
  MainMenu() = default;
  ~MainMenu(){};
  void init(IRenderer *renderer);
  void render(int winWidth, int winHeight, IRenderer *renderer);
  void process(MenuState *currentState, IRenderer *renderer);

private:
  void *font;
  void *planet = nullptr;

  std::array<std::string, 4> mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
  int currentMenuIndex = 0;
};