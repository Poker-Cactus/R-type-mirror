/**
 * @file Geometry.hpp
 * @brief Basic geometric primitives for rendering and collision.
 */

#pragma once

/**
 * @struct Point
 * @brief Simple 2D point with integer coordinates.
 */
struct Point {
  int x; ///< X coordinate.
  int y; ///< Y coordinate.
};

/**
 * @struct Rect
 * @brief Axis-aligned rectangle defined by position and size.
 */
struct Rect {
  int x; ///< Left coordinate.
  int y; ///< Top coordinate.
  int width; ///< Width in pixels.
  int height; ///< Height in pixels.
};

/**
 * @struct Circle
 * @brief Circle defined by center point and radius.
 */
struct Circle {
  int centerX; ///< Center X coordinate.
  int centerY; ///< Center Y coordinate.
  int radius; ///< Radius in pixels.
};
