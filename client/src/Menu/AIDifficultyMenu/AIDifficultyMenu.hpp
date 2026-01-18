/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AIDifficultyMenu.hpp
*/
/**
 * @file AIDifficultyMenu.hpp
 * @brief AI difficulty selection menu interface
 */

#pragma once
#include "../../../include/Settings.hpp"
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>

class Settings;

/**
 * @class AIDifficultyMenu
 * @brief Menu for selecting AI difficulty in solo mode
 *
 * Displays and handles the AI difficulty selection with options:
 * Weak, Medium, Strong, Back
 */
class AIDifficultyMenu
{
public:
  AIDifficultyMenu(std::shared_ptr<IRenderer> renderer, std::function<void(AIDifficulty)> onDifficultySelected);
  ~AIDifficultyMenu();

  /**
   * @brief Initialize menu resources
   */
  void init();

  /**
   * @brief Render the AI difficulty menu
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process user input
   * @param currentState Pointer to current menu state
   * @param settings Settings reference for storing AI difficulty
   */
  void process(MenuState *currentState, Settings &settings);

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *font;

  std::array<std::string, 5> aiDifficultyItems = {"Weak", "Medium", "Strong", "No Ally", "Back"};
  int currentMenuIndex = 0;

  std::function<void(AIDifficulty)> m_onDifficultySelected;

  void *clickedSound;
  void *hoverSound;
};
