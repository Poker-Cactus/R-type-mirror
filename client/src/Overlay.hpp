/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Overlay.hpp
*/

/**
 * @file Overlay.hpp
 * @brief Screen overlay rendering
 */

#pragma once
#include "../interface/IRenderer.hpp"
#include <cstdint>
#include <memory>

/**
 * @struct ColorRGBA
 * @brief RGBA color representation
 */
struct ColorRGBA {
  std::uint8_t red; ///< Red component (0-255)
  std::uint8_t green; ///< Green component (0-255)
  std::uint8_t blue; ///< Blue component (0-255)
  std::uint8_t alpha; ///< Alpha transparency (0-255)
};

/**
 * @class Overlay
 * @brief Manages a colored full-screen overlay with transparency
 */
class Overlay
{
public:
  static constexpr std::uint8_t DEFAULT_ALPHA = 128; ///< Default transparency

  /**
   * @brief Construct overlay with black color and specified transparency
   * @param renderer Renderer interface
   * @param alpha Transparency (0=fully transparent, 255=fully opaque)
   */
  Overlay(std::shared_ptr<IRenderer> renderer, std::uint8_t alpha = DEFAULT_ALPHA);

  /**
   * @brief Construct overlay with specified color
   * @param renderer Renderer interface
   * @param color RGBA color
   */
  Overlay(std::shared_ptr<IRenderer>, const ColorRGBA &color);

  /**
   * @brief Set overlay color
   * @param color RGBA color
   */
  void setColor(const ColorRGBA &color);

  /**
   * @brief Set overlay transparency
   * @param alpha Transparency level (0-255)
   */
  void setAlpha(std::uint8_t alpha);

  /**
   * @brief Render overlay covering entire screen
   */
  void render();

private:
  std::shared_ptr<IRenderer> m_renderer;
  std::uint8_t m_red;
  std::uint8_t m_green;
  std::uint8_t m_blue;
  std::uint8_t m_alpha;
};
