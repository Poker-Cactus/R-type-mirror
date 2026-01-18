/**
 * @file Color.hpp
 * @brief RGBA color structure.
 */

#pragma once

/**
 * @struct Color
 * @brief RGBA color with 8-bit channels.
 */
struct Color {
  unsigned char r = 0; ///< Red channel.
  unsigned char g = 0; ///< Green channel.
  unsigned char b = 0; ///< Blue channel.
  unsigned char a = 255; ///< Alpha channel.
};
