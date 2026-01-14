/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Settings implementation
*/

#include "Settings.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

bool Settings::saveToFile()
{
  try {
    nlohmann::json settingsJson;

    // Audio settings
    settingsJson["audio"]["masterVolume"] = masterVolume;
    settingsJson["audio"]["musicVolume"] = musicVolume;
    settingsJson["audio"]["sfxVolume"] = sfxVolume;

    // Control settings
    settingsJson["controls"]["up"] = up;
    settingsJson["controls"]["down"] = down;
    settingsJson["controls"]["left"] = left;
    settingsJson["controls"]["right"] = right;
    settingsJson["controls"]["shoot"] = shoot;
    settingsJson["controls"]["chargedShoot"] = chargedShoot;

    // Graphics settings
    settingsJson["graphics"]["fullScreen"] = fullScreen;

    // Debug settings
    settingsJson["debug"]["showInfoMode"] = showInfoMode;
    settingsJson["debug"]["showCPUUsage"] = showCPUUsage;
    settingsJson["debug"]["showRAMUsage"] = showRAMUsage;
    settingsJson["debug"]["showFPS"] = showFPS;
    settingsJson["debug"]["showEntityCount"] = showEntityCount;
    settingsJson["debug"]["showNetworkInfo"] = showNetworkInfo;

    // Profile settings
    settingsJson["profile"]["username"] = username;

    std::ofstream file("settings.json");
    if (!file.is_open()) {
      std::cerr << "[Settings] Failed to open settings.json for writing" << std::endl;
      return false;
    }

    file << settingsJson.dump(2);
    file.close();

    std::cout << "[Settings] Settings saved successfully" << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[Settings] Error saving settings: " << e.what() << std::endl;
    return false;
  }
}

bool Settings::loadFromFile()
{
  try {
    std::ifstream file("settings.json");
    if (!file.is_open()) {
      std::cout << "[Settings] No settings file found, using defaults" << std::endl;
      return false;
    }

    nlohmann::json settingsJson;
    file >> settingsJson;
    file.close();

    // Audio settings
    if (settingsJson.contains("audio")) {
      masterVolume = settingsJson["audio"].value("masterVolume", masterVolume);
      musicVolume = settingsJson["audio"].value("musicVolume", musicVolume);
      sfxVolume = settingsJson["audio"].value("sfxVolume", sfxVolume);
    }

    // Control settings
    if (settingsJson.contains("controls")) {
      up = settingsJson["controls"].value("up", up);
      down = settingsJson["controls"].value("down", down);
      left = settingsJson["controls"].value("left", left);
      right = settingsJson["controls"].value("right", right);
      shoot = settingsJson["controls"].value("shoot", shoot);
      chargedShoot = settingsJson["controls"].value("chargedShoot", chargedShoot);
    }

    // Graphics settings
    if (settingsJson.contains("graphics")) {
      fullScreen = settingsJson["graphics"].value("fullScreen", fullScreen);
    }

    // Debug settings
    if (settingsJson.contains("debug")) {
      showInfoMode = settingsJson["debug"].value("showInfoMode", showInfoMode);
      showCPUUsage = settingsJson["debug"].value("showCPUUsage", showCPUUsage);
      showRAMUsage = settingsJson["debug"].value("showRAMUsage", showRAMUsage);
      showFPS = settingsJson["debug"].value("showFPS", showFPS);
      showEntityCount = settingsJson["debug"].value("showEntityCount", showEntityCount);
      showNetworkInfo = settingsJson["debug"].value("showNetworkInfo", showNetworkInfo);
    }

    // Profile settings
    if (settingsJson.contains("profile")) {
      username = settingsJson["profile"].value("username", username);
    }

    std::cout << "[Settings] Settings loaded successfully" << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[Settings] Error loading settings: " << e.what() << std::endl;
    return false;
  }
}
