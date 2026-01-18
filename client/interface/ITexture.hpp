/**
 * @file ITexture.hpp
 * @brief Texture loading and rendering interface.
 */

#pragma once
#include "Geometry.hpp"
#include <string>

/**
 * @class ITexture
 * @brief Interface for texture operations.
 */
class ITexture
{
public:
  virtual ~ITexture() = default;

  /** @brief Load a texture from a file path. */
  virtual void *loadTexture(const std::string &filepath) = 0;
  /** @brief Release a texture resource. */
  virtual void freeTexture(void *texture) = 0;
  /** @brief Query texture dimensions. */
  virtual void getTextureSize(void *texture, int &width, int &height) = 0;
  /** @brief Draw a texture at a position. */
  virtual void drawTexture(void *texture, int pos_x, int pos_y) = 0;
  /** @brief Draw a portion of a texture into a destination rectangle. */
  virtual void drawTextureRegion(void *texture, const Rect &src, const Rect &dst) = 0;
  /** @brief Draw a texture region with rotation and flip. */
  virtual void drawTextureRegionEx(void *texture, const Rect &src, const Rect &dst, double angle, bool flipX,
                                   bool flipY) = 0;
  /** @brief Draw a texture with scaling, rotation, and flip. */
  virtual void drawTextureEx(void *texture, int posX, int posY, int width, int height, double angle, bool flipX,
                             bool flipY) = 0;
};
