/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IntroScreen.hpp
*/

/**
 * @file IntroScreen.hpp
 * @brief Intro screen with moon zoom animation
 */

#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <memory>

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
  IntroScreen(std::shared_ptr<IRenderer> renderer);
  ~IntroScreen();

  /**
   * @brief Initialize intro screen resources
   */
  void init();

  /**
   * @brief Render the intro screen
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process user input
   * @return true if should transition to main menu
   */
  bool process();

  /**
   * @brief Check if intro is complete
   * @return true if animation finished
   */
  bool isComplete() const { return m_isComplete; }

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *m_backgroundTexture = nullptr; ///< Starfield background
  void *m_font = nullptr; ///< Font for text
  void *m_planet = nullptr; ///< Moon texture

  float m_blinkTimer = 0.0f; ///< Timer for text blink effect
  bool m_isZooming = false; ///< Whether zoom animation is active
  float m_zoomTimer = 0.0f; ///< Zoom animation timer
  float m_zoomScale = 0.3f; ///< Current zoom scale
  float m_zoomDuration = 0.8f; ///< Zoom animation duration
  float m_backgroundOffsetX = 0.0f; ///< Background scroll offset
  bool m_isComplete = false; ///< Whether intro is complete

  void *m_music; ///< Main music theme for the menu
};
