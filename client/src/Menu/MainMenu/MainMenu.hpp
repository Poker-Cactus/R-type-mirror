/**
 * @file MainMenu.hpp
 * @brief Main menu screen interface
 */

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <string>

/**
 * @class MainMenu
 * @brief Main menu with navigation options
 *
 * Displays and handles the primary menu with options:
 * Play, Settings, Profile, Exit
 */
class MainMenu
{
public:
  MainMenu() = default;
  ~MainMenu() {}

  /**
   * @brief Initialize menu resources
   * @param renderer Renderer interface
   */
  void init(IRenderer *renderer);

  /**
   * @brief Render the main menu
   * @param winWidth Window width
   * @param winHeight Window height
   * @param renderer Renderer interface
   */
  void render(int winWidth, int winHeight, IRenderer *renderer);

  /**
   * @brief Process user input
   * @param currentState Pointer to current menu state
   * @param renderer Renderer interface
   */
  void process(MenuState *currentState, IRenderer *renderer);

private:
  void *font;
  void *planet = nullptr;

  std::array<std::string, 4> mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
  int currentMenuIndex = 0;
};