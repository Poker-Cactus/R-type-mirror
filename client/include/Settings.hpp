/**
 * @file Settings.hpp
 * @brief Game settings configuration
 */

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "../interface/KeyCodes.hpp"
#include <string>

/**
 * @class Settings
 * @brief Manages user preferences and game settings
 *
 * This class stores all configurable settings including audio levels,
 * input controls, and graphics options.
 */
class Settings
{
public:
  Settings() = default;
  ~Settings() = default;

  /**
   * @brief Save settings to a file
   * @return true if save was successful
   */
  bool saveToFile();

  /**
   * @brief Load settings from a file
   * @return true if load was successful
   */
  bool loadFromFile();

  // AUDIO Settings
  int masterVolume = 100; ///< Master volume level (0-100)
  int musicVolume = 100;  ///< Music volume level (0-100)
  int sfxVolume = 100;    ///< Sound effects volume level (0-100)

  // CONTROL Settings
  int up = KeyCode::KEY_UP;            ///< Key binding for moving up
  int down = KeyCode::KEY_DOWN;        ///< Key binding for moving down
  int left = KeyCode::KEY_LEFT;        ///< Key binding for moving left
  int right = KeyCode::KEY_RIGHT;      ///< Key binding for moving right
  int shoot = KeyCode::KEY_SPACE;      ///< Key binding for shooting
  int chargedShoot = KeyCode::KEY_E;   ///< Key binding for charged shot

  // GRAPHIC Settings
  bool fullScreen = true; ///< Fullscreen mode toggle

  // PROFILE Settings
  std::string username = "Player"; ///< Player username

private:
};

#endif
