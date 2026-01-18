/**
 * @file PauseMenu.hpp
 * @brief Pause menu overlay during gameplay
 */

#pragma once

#include "../../../interface/IRenderer.hpp"
#include <memory>
#include <string>
#include <vector>

/**
 * @enum PauseMenuAction
 * @brief Actions available in the pause menu
 */
enum class PauseMenuAction {
  NONE, ///< No action selected
  RESUME, ///< Resume the game
  SETTINGS, ///< Open settings from pause menu
  QUIT ///< Quit to main menu
};

/**
 * @class PauseMenu
 * @brief Simple pause menu overlay
 *
 * Provides a minimal menu with Resume and Quit options
 * when the game is paused.
 */
class PauseMenu
{
public:
  /**
   * @brief Construct pause menu
   * @param renderer Renderer interface
   */
  explicit PauseMenu(std::shared_ptr<IRenderer> renderer);
  ~PauseMenu();

  /**
   * @brief Initialize pause menu resources
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Render the pause menu
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process input and return selected action
   * @return Action selected by the user
   */
  PauseMenuAction process();

  /**
   * @brief Reset menu state (selection to Resume)
   */
  void reset();

private:
  std::shared_ptr<IRenderer> m_renderer; ///< Renderer interface
  void *m_font = nullptr; ///< Menu font
  void *m_titleFont = nullptr; ///< Title font
  int m_selectedIndex = 0; ///< Currently selected option
  std::vector<std::string> m_options; ///< Menu options
  void *m_selectSound = nullptr; ///< Selection sound
  void *m_confirmSound = nullptr; ///< Confirm sound
};
