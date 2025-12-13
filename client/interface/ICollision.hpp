#pragma once

class ICollision
{
  public:
    virtual ~ICollision() = default;

    virtual bool checkCollisionRects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) = 0;
    virtual bool checkCollisionCircles(int x1, int y1, int r1, int x2, int y2, int r2) = 0;
    virtual bool checkPointInRect(int px, int py, int rx, int ry, int rw, int rh) = 0;
};
