/**
 * @file ProfileMenu.hpp
 * @brief Player profile menu screen
 */

#pragma once
#include "../../../interface/IRenderer.hpp"

/**
 * @class ProfileMenu
 * @brief Displays and manages player profile information
 */
class ProfileMenu
{
public:
  ProfileMenu() = default;
  ~ProfileMenu() {}

  /**
   * @brief Initialize profile menu resources
   * @param renderer Renderer interface
   */
  void init(IRenderer *renderer);

  /**
   * @brief Render the profile menu
   * @param winWidth Window width
   * @param winHeight Window height
   * @param renderer Renderer interface
   */
  void render(int winWidth, int winHeight, IRenderer *renderer);

  /**
   * @brief Process user input
   * @param renderer Renderer interface
   */
  void process(IRenderer *renderer);

private:
};