/**
 * @file IText.hpp
 * @brief Text rendering interface.
 */

#pragma once
#include "Color.hpp"
#include <string>

/**
 * @class IText
 * @brief Interface for font loading and text rendering.
 */
class IText
{
public:
  virtual ~IText() = default;

  /** @brief Load a font at a given size. */
  virtual void *loadFont(const std::string &filepath, int fontSize) = 0;
  /** @brief Release a font resource. */
  virtual void freeFont(void *font) = 0;
  /** @brief Draw text at a position. */
  virtual void drawText(void *font, const std::string &text, int x, int y,
                        const Color &color = {255, 255, 255, 255}) = 0;
  /** @brief Measure text dimensions for a font. */
  virtual void getTextSize(void *font, const std::string &text, int &width, int &height) = 0;
};
