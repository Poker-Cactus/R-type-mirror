/**
 * @file IAudio.hpp
 * @brief Audio backend abstraction.
 */

#pragma once
#include <string>

/**
 * @class IAudio
 * @brief Interface for loading and playing audio resources.
 */
class IAudio
{
public:
  virtual ~IAudio() = default;

  /** @brief Load a sound effect file. */
  virtual void *loadSound(const std::string &filepath) = 0;
  /** @brief Load a music track file. */
  virtual void *loadMusic(const std::string &filepath) = 0;
  /** @brief Play a sound effect. */
  virtual void playSound(void *sound, int loops = 0) = 0;
  /** @brief Play a music track. */
  virtual void playMusic(void *music, int loops = -1) = 0;
  /** @brief Pause the current music track. */
  virtual void pauseMusic() = 0;
  /** @brief Resume paused music playback. */
  virtual void resumeMusic() = 0;
  /** @brief Stop the current music track. */
  virtual void stopMusic() = 0;
  /** @brief Set the global sound effect volume. */
  virtual void setSoundVolume(int volume) = 0;
  /** @brief Set the global music volume. */
  virtual void setMusicVolume(int volume) = 0;
  /** @brief Release a sound effect resource. */
  virtual void freeSound(void *sound) = 0;
  /** @brief Release a music track resource. */
  virtual void freeMusic(void *music) = 0;
  /** @brief Check whether music is currently playing. */
  virtual bool isMusicPlaying() = 0;
};
