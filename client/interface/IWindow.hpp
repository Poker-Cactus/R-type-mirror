/**
 * @file IWindow.hpp
 * @brief Window management interface.
 */

#pragma once
#include <string>

/**
 * @class IWindow
 * @brief Interface for window size and state management.
 */
class IWindow
{
public:
  virtual ~IWindow() = default;

  /** @brief Get the window width in pixels. */
  virtual int getWindowWidth() const = 0;
  /** @brief Get the window height in pixels. */
  virtual int getWindowHeight() const = 0;
  /** @brief Set the window size in pixels. */
  virtual void setWindowSize(int width, int height) = 0;
  /** @brief Set the window title. */
  virtual void setWindowTitle(const std::string &title) = 0;
  /** @brief Enable or disable fullscreen mode. */
  virtual void setFullscreen(bool fullscreen) = 0;
  /** @brief Check whether fullscreen mode is active. */
  virtual bool isFullscreen() const = 0;
  /** @brief Poll and process window events. */
  virtual bool pollEvents() = 0;
};
