/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Settings.hpp
*/

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "../interface/KeyCodes.hpp"

class Settings
{
public:
  Settings() = default;
  ~Settings() = default;
  // AUDIO Settings
  int masterVolume = 100;
  int musicVolume = 100;
  int sfxVolume = 100;
  // CONTROL Settings
  int up = KeyCode::KEY_UP;
  int down = KeyCode::KEY_DOWN;
  int left = KeyCode::KEY_LEFT;
  int right = KeyCode::KEY_RIGHT;
  int shoot = KeyCode::KEY_SPACE;
  int chargedShoot = KeyCode::KEY_E;
  // GRAPHIC Settings
  bool fullScreen = true;

private:
};

#endif