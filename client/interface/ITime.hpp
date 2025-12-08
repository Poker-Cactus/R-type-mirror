#pragma once

class ITime
{
  public:
    virtual ~ITime() = default;

    virtual float getDeltaTime() const = 0;
    virtual int getFPS() const = 0;
    virtual void setTargetFPS(int fps) = 0;
    virtual void setVSync(bool enabled) = 0;
};
