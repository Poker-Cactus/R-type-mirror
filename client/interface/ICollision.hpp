/**
 * @file ICollision.hpp
 * @brief Collision query interface.
 */

#pragma once
#include "Geometry.hpp"

/**
 * @class ICollision
 * @brief Interface for basic collision checks.
 */
class ICollision
{
public:
  virtual ~ICollision() = default;

  /** @brief Check overlap between two axis-aligned rectangles. */
  virtual bool checkCollisionRects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) = 0;
  /** @brief Check overlap between two circles. */
  virtual bool checkCollisionCircles(const Circle &circle1, const Circle &circle2) = 0;
  /** @brief Check whether a point lies within a rectangle. */
  virtual bool checkPointInRect(int px, int py, int rx, int ry, int rw, int rh) = 0;
};
