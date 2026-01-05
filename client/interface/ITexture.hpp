#pragma once
#include "Geometry.hpp"
#include <string>

class ITexture
{
public:
  virtual ~ITexture() = default;

  virtual void *loadTexture(const std::string &filepath) = 0;
  virtual void freeTexture(void *texture) = 0;
  virtual void getTextureSize(void *texture, int &width, int &height) = 0;
  virtual void drawTexture(void *texture, int pos_x, int pos_y) = 0;
  virtual void drawTextureRegion(void *texture, const Rect &src, const Rect &dst) = 0;
  virtual void drawTextureEx(void *texture, int posX, int posY, int width, int height, double angle, bool flipX, bool flipY) = 0;
};
