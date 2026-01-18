/**
 * @file IColorBlindSupport.hpp
 * @brief Interface for color-blind accessibility support.
 */

#pragma once

#include "../include/ColorBlindFilter.hpp"

/**
 * @class IColorBlindSupport
 * @brief Adds color-blind mode configuration for renderers.
 */
class IColorBlindSupport
{
public:
  virtual ~IColorBlindSupport() = default;

  /**
   * @brief Set the active color-blind mode.
   * @param mode Color-blind mode to apply.
   */
  virtual void setColorBlindMode(ColorBlindMode mode) = 0;

  /**
   * @brief Get the currently active color-blind mode.
   * @return Active color-blind mode.
   */
  virtual ColorBlindMode getColorBlindMode() const = 0;
};
