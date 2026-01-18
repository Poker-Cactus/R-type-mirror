/**
 * @file IRender.hpp
 * @brief Basic render loop operations.
 */

#pragma once
#include "Color.hpp"

/**
 * @class IRender
 * @brief Interface for frame clearing and presenting.
 */
class IRender
{
public:
  virtual ~IRender() = default;

  /** @brief Clear the current frame with a color. */
  virtual void clear(const Color &color = {0, 0, 0, 255}) = 0;
  /** @brief Present the rendered frame to the window. */
  virtual void present() = 0;
};