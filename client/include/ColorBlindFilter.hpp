/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ColorBlindFilter.hpp
*/

#ifndef COLOR_BLIND_FILTER_HPP
#define COLOR_BLIND_FILTER_HPP

#include <cstdint>

enum class ColorBlindMode : std::uint8_t {
  NONE = 0,
  PROTANOPIA = 1,
  DEUTERANOPIA = 2,
  TRITANOPIA = 3,
};

struct ColorMatrix {
  float m[3][3];
};

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
