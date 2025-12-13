#pragma once
#include "Color.hpp"

class IShape
{
public:
  virtual ~IShape() = default;

  virtual void drawRect(int x, int y, int w, int h, const Color &color = {255, 255, 255, 255}) = 0;
  virtual void drawRectOutline(int x, int y, int w, int h, const Color &color) = 0;
  virtual void drawLine(int x1, int y1, int x2, int y2, const Color &color) = 0;
  virtual void drawCircle(int centerX, int centerY, int radius, const Color &color) = 0;
  virtual void drawCircleFilled(int centerX, int centerY, int radius, const Color &color) = 0;
  virtual void drawPoint(int x, int y, const Color &color) = 0;
};
