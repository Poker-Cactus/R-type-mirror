#pragma once
#include <string>

class ITexture
{
public:
  virtual ~ITexture() = default;

  virtual void *loadTexture(const std::string &filepath) = 0;
  virtual void freeTexture(void *texture) = 0;
  virtual void getTextureSize(void *texture, int &width, int &height) = 0;
  virtual void drawTexture(void *texture, int x, int y) = 0;
  virtual void drawTextureRegion(void *texture, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY, int dstW,
                                 int dstH) = 0;
  virtual void drawTextureEx(void *texture, int x, int y, int w, int h, double angle, bool flipX = false,
                             bool flipY = false) = 0;
};
