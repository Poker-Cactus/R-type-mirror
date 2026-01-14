/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** MainMenu.hpp
*/
/**
 * @file MainMenu.hpp
 * @brief Main menu screen interface
 */

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <memory>
#include <string>

class Settings;

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
  MainMenu(std::shared_ptr<IRenderer> renderer);
  ~MainMenu();

  /**
   * @brief Initialize menu resources
   */
  void init();

  /**
   * @brief Render the main menu
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process user input
   * @param currentState Pointer to current menu state
   * @param settings Settings reference for key bindings
   */
  void process(MenuState *currentState, Settings &settings);

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *font;
  void *planet;

  std::array<std::string, 5> mainMenuItems = {"Play", "Settings", "Profile", "Asset Editor", "Exit"};
  int currentMenuIndex = 0;

  void *clickedSound;
  void *hoverSound;

#ifndef _WIN32
  pid_t assetEditorPid = -1; // Track Asset Editor process ID
#endif
};
