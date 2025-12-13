#pragma once
#include <string>

class IWindow
{
public:
  virtual ~IWindow() = default;

  virtual int getWindowWidth() const = 0;
  virtual int getWindowHeight() const = 0;
  virtual void setWindowSize(int width, int height) = 0;
  virtual void setWindowTitle(const std::string &title) = 0;
  virtual void setFullscreen(bool fullscreen) = 0;
  virtual bool isFullscreen() const = 0;
  virtual bool pollEvents() = 0;
};
