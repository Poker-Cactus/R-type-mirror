/**
 * @file ICamera.hpp
 * @brief Camera and viewport controls.
 */

#pragma once

/**
 * @class ICamera
 * @brief Interface for camera offset and viewport management.
 */
class ICamera
{
public:
  virtual ~ICamera() = default;

  /** @brief Set the render viewport in window coordinates. */
  virtual void setViewport(int x, int y, int w, int h) = 0;
  /** @brief Reset the viewport to the full window. */
  virtual void resetViewport() = 0;
  /** @brief Set camera offset applied to rendering. */
  virtual void setCameraOffset(int offsetX, int offsetY) = 0;
  /** @brief Get the current camera offset. */
  virtual void getCameraOffset(int &offsetX, int &offsetY) const = 0;
};
