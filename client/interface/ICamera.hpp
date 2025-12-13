#pragma once

class ICamera
{
  public:
    virtual ~ICamera() = default;

    virtual void setViewport(int x, int y, int w, int h) = 0;
    virtual void resetViewport() = 0;
    virtual void setCameraOffset(int offsetX, int offsetY) = 0;
    virtual void getCameraOffset(int &offsetX, int &offsetY) const = 0;
};
