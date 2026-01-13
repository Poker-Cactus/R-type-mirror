/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ProfileMenu.hpp
*/
/**
 * @file ProfileMenu.hpp
 * @brief Player profile menu screen
 */

#pragma once
#include "../../../include/Settings.hpp"
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <memory>

/**
 * @class ProfileMenu
 * @brief Displays and manages player profile information
 */
class ProfileMenu
{
public:
  ProfileMenu(std::shared_ptr<IRenderer> renderer);
  ~ProfileMenu();

  /**
   * @brief Initialize profile menu resources
   * @param renderer Renderer interface
   * @param settings Game settings reference
   */
  void init(Settings &settings);

  /**
   * @brief Render the profile menu
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

  /**
   * @brief Check if the profile menu is currently in editing mode
   * @return true if editing username
   */
  [[nodiscard]] bool isEditing() const { return isEditingUsername; }

private:
  Settings *m_settings = nullptr;
  void *font = nullptr;
  void *titleFont = nullptr;
  std::string currentUsername;
  bool isEditingUsername = false;
  int selectedOption = 0; // 0 = username, 1 = save
  std::shared_ptr<IRenderer> m_renderer;
};
