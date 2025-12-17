#pragma once
#include "Geometry.hpp"

class ICollision
{
public:
  virtual ~ICollision() = default;

  virtual bool checkCollisionRects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) = 0;
  virtual bool checkCollisionCircles(const Circle &circle1, const Circle &circle2) = 0;
  virtual bool checkPointInRect(int px, int py, int rx, int ry, int rw, int rh) = 0;
};
