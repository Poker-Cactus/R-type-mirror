/**
 * @file IntroScreen.hpp
 * @brief Intro screen with moon zoom animation
 */

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"

/**
 * @class IntroScreen
 * @brief Displays animated intro screen with moon zoom effect
 *
 * Shows a zooming moon animation with "Press enter to start" prompt.
 * Transitions to main menu when animation completes or user presses enter.
 */
class IntroScreen
{
public:
  IntroScreen() = default;
  ~IntroScreen() {}

  /**
   * @brief Initialize intro screen resources
   * @param renderer Renderer interface
   */
  void init(IRenderer *renderer);

  /**
   * @brief Render the intro screen
   * @param winWidth Window width
   * @param winHeight Window height
   * @param renderer Renderer interface
   */
  void render(int winWidth, int winHeight, IRenderer *renderer);

  /**
   * @brief Process user input
   * @param renderer Renderer interface
   * @return true if should transition to main menu
   */
  bool process(IRenderer *renderer);

  /**
   * @brief Check if intro is complete
   * @return true if animation finished
   */
  bool isComplete() const { return m_isComplete; }

private:
  void *m_backgroundTexture = nullptr; ///< Starfield background
  void *m_font = nullptr;              ///< Font for text
  void *m_planet = nullptr;            ///< Moon texture

  float m_blinkTimer = 0.0f;      ///< Timer for text blink effect
  bool m_isZooming = false;       ///< Whether zoom animation is active
  float m_zoomTimer = 0.0f;       ///< Zoom animation timer
  float m_zoomScale = 0.3f;       ///< Current zoom scale
  float m_zoomDuration = 0.8f;    ///< Zoom animation duration
  float m_backgroundOffsetX = 0.0f; ///< Background scroll offset
  bool m_isComplete = false;      ///< Whether intro is complete
};
