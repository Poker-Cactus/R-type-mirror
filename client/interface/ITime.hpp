/**
 * @file ITime.hpp
 * @brief Timing and frame pacing interface.
 */

#pragma once

/**
 * @class ITime
 * @brief Interface for time and frame synchronization.
 */
class ITime
{
public:
  virtual ~ITime() = default;

  /** @brief Get the time elapsed since the last frame. */
  virtual float getDeltaTime() const = 0;
  /** @brief Get the current frames per second. */
  virtual int getFPS() const = 0;
  /** @brief Set the target frame rate. */
  virtual void setTargetFPS(int fps) = 0;
  /** @brief Enable or disable vertical sync. */
  virtual void setVSync(bool enabled) = 0;
};
