/**
 * @file IShape.hpp
 * @brief Primitive shape drawing interface.
 */

#pragma once
#include "Color.hpp"
#include "Geometry.hpp"

/**
 * @class IShape
 * @brief Interface for drawing geometric primitives.
 */
class IShape
{
public:
  virtual ~IShape() = default;

  /** @brief Draw a filled rectangle. */
  virtual void drawRect(int x, int y, int w, int h, const Color &color = {255, 255, 255, 255}) = 0;
  /** @brief Draw a rectangle outline. */
  virtual void drawRectOutline(int x, int y, int w, int h, const Color &color) = 0;
  /** @brief Draw a line segment. */
  virtual void drawLine(int x1, int y1, int x2, int y2, const Color &color) = 0;
  /** @brief Draw a circle outline. */
  virtual void drawCircle(const Circle &circle, const Color &color) = 0;
  /** @brief Draw a filled circle. */
  virtual void drawCircleFilled(const Circle &circle, const Color &color) = 0;
  /** @brief Draw a point. */
  virtual void drawPoint(int x, int y, const Color &color) = 0;
};
