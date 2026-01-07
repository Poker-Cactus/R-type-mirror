/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** MainMenu.hpp
*/

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <string>
#include <memory>

class MainMenu
{
public:
  MainMenu(std::shared_ptr<IRenderer> renderer);
  ~MainMenu();
  void init();
  void render(int winWidth, int winHeight);
  void process(MenuState *currentState);

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *font;
  void *planet;

  std::array<std::string, 4> mainMenuItems;
  int currentMenuIndex;
};
