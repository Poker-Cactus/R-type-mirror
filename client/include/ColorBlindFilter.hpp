/**
 * @file ColorBlindFilter.hpp
 * @brief Color-blind filter matrices and helpers.
 */

#ifndef COLOR_BLIND_FILTER_HPP
#define COLOR_BLIND_FILTER_HPP

#include <cstdint>

/**
 * @enum ColorBlindMode
 * @brief Supported color-blind filter modes.
 */
enum class ColorBlindMode : std::uint8_t {
  NONE = 0, ///< No filter applied.
  PROTANOPIA = 1, ///< Red-blindness simulation.
  DEUTERANOPIA = 2, ///< Green-blindness simulation.
  TRITANOPIA = 3, ///< Blue-blindness simulation.
};

/**
 * @struct ColorMatrix
 * @brief 3x3 RGB color transform matrix.
 */
struct ColorMatrix {
  float m[3][3]; ///< Matrix values in row-major order.
};

/**
 * @brief Get the color transform matrix for a given mode.
 * @param mode Color-blind mode.
 * @return RGB transformation matrix.
 */
inline ColorMatrix getColorBlindMatrix(ColorBlindMode mode)
{
  ColorMatrix matrix;

  switch (mode) {
  case ColorBlindMode::NONE:
    matrix.m[0][0] = 1.0f;
    matrix.m[0][1] = 0.0f;
    matrix.m[0][2] = 0.0f;
    matrix.m[1][0] = 0.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[1][2] = 0.0f;
    matrix.m[2][0] = 0.0f;
    matrix.m[2][1] = 0.0f;
    matrix.m[2][2] = 1.0f;
    break;

  case ColorBlindMode::PROTANOPIA:
    matrix.m[0][0] = 0.567f;
    matrix.m[0][1] = 0.433f;
    matrix.m[0][2] = 0.0f;
    matrix.m[1][0] = 0.558f;
    matrix.m[1][1] = 0.442f;
    matrix.m[1][2] = 0.0f;
    matrix.m[2][0] = 0.0f;
    matrix.m[2][1] = 0.242f;
    matrix.m[2][2] = 0.758f;
    break;

  case ColorBlindMode::DEUTERANOPIA:
    matrix.m[0][0] = 0.625f;
    matrix.m[0][1] = 0.375f;
    matrix.m[0][2] = 0.0f;
    matrix.m[1][0] = 0.7f;
    matrix.m[1][1] = 0.3f;
    matrix.m[1][2] = 0.0f;
    matrix.m[2][0] = 0.0f;
    matrix.m[2][1] = 0.3f;
    matrix.m[2][2] = 0.7f;
    break;

  case ColorBlindMode::TRITANOPIA:
    matrix.m[0][0] = 0.95f;
    matrix.m[0][1] = 0.05f;
    matrix.m[0][2] = 0.0f;
    matrix.m[1][0] = 0.0f;
    matrix.m[1][1] = 0.433f;
    matrix.m[1][2] = 0.567f;
    matrix.m[2][0] = 0.0f;
    matrix.m[2][1] = 0.475f;
    matrix.m[2][2] = 0.525f;
    break;
  }

  return matrix;
}

/**
 * @brief Apply a color-blind filter matrix to an RGB triplet.
 * @param r Red channel (in/out).
 * @param g Green channel (in/out).
 * @param b Blue channel (in/out).
 * @param matrix Color transformation matrix.
 */
inline void applyColorBlindFilter(std::uint8_t &r, std::uint8_t &g, std::uint8_t &b, const ColorMatrix &matrix)
{
  float fr = static_cast<float>(r) / 255.0f;
  float fg = static_cast<float>(g) / 255.0f;
  float fb = static_cast<float>(b) / 255.0f;

  float newR = fr * matrix.m[0][0] + fg * matrix.m[0][1] + fb * matrix.m[0][2];
  float newG = fr * matrix.m[1][0] + fg * matrix.m[1][1] + fb * matrix.m[1][2];
  float newB = fr * matrix.m[2][0] + fg * matrix.m[2][1] + fb * matrix.m[2][2];

  newR = newR < 0.0f ? 0.0f : (newR > 1.0f ? 1.0f : newR);
  newG = newG < 0.0f ? 0.0f : (newG > 1.0f ? 1.0f : newG);
  newB = newB < 0.0f ? 0.0f : (newB > 1.0f ? 1.0f : newB);

  r = static_cast<std::uint8_t>(newR * 255.0f);
  g = static_cast<std::uint8_t>(newG * 255.0f);
  b = static_cast<std::uint8_t>(newB * 255.0f);
}

/**
 * @brief Get a human-readable name for a color-blind mode.
 * @param mode Color-blind mode.
 * @return Constant string name.
 */
inline const char *getColorBlindModeName(ColorBlindMode mode)
{
  switch (mode) {
  case ColorBlindMode::NONE:
    return "Normal";
  case ColorBlindMode::PROTANOPIA:
    return "Protanopia";
  case ColorBlindMode::DEUTERANOPIA:
    return "Deuteranopia";
  case ColorBlindMode::TRITANOPIA:
    return "Tritanopia";
  default:
    return "Unknown";
  }
}

#endif // COLOR_BLIND_FILTER_HPP
