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
#include "../../../interface/IRenderer.hpp"
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
   */
  void init();

  /**
   * @brief Render the profile menu
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process user input
   */
  void process();

private:
  std::shared_ptr<IRenderer> m_renderer;
};
