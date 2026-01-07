/**
 * @file ProfileMenu.hpp
 * @brief Player profile menu screen
 */

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../../../include/Settings.hpp"

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
   * @param settings Game settings reference
   */
  void init(IRenderer *renderer, Settings &settings);

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

  /**
   * @brief Check if the profile menu is currently in editing mode
   * @return true if editing username
   */
  [[nodiscard]] bool isEditing() const { return isEditingUsername; }

private:
  Settings *settings = nullptr;
  void *font = nullptr;
  void *titleFont = nullptr;
  std::string currentUsername;
  bool isEditingUsername = false;
  int selectedOption = 0; // 0 = username, 1 = save
};
