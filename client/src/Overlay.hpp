/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Overlay.hpp
*/

#pragma once
#include "../interface/IRenderer.hpp"
#include <cstdint>

struct ColorRGBA {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  std::uint8_t alpha;
};

/**
 * @brief Manages a colored overlay with configurable transparency
 */
class Overlay
{
public:
  static constexpr std::uint8_t DEFAULT_ALPHA = 128;

  /**
   * @brief Constructs an overlay with black color and default transparency
   * @param renderer Pointer to the renderer interface
   * @param alpha Transparency (0=fully transparent, 255=fully opaque)
   */
  Overlay(IRenderer *renderer, std::uint8_t alpha = DEFAULT_ALPHA);

  /**
   * @brief Constructs an overlay with specified color
   * @param renderer Pointer to the renderer interface
   * @param color Color with RGBA components
   */
  Overlay(IRenderer *renderer, const ColorRGBA &color);

  /**
   * @brief Sets the overlay color
   */
  void setColor(const ColorRGBA &color);

  /**
   * @brief Sets the overlay transparency
   * @param alpha Transparency level (0-255)
   */
  void setAlpha(std::uint8_t alpha);

  /**
   * @brief Renders the overlay covering the entire screen
   */
  void render();

private:
  IRenderer *m_renderer;
  std::uint8_t m_red;
  std::uint8_t m_green;
  std::uint8_t m_blue;
  std::uint8_t m_alpha;
};
