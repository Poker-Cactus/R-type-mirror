/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** ProcessInputMenu.cpp
*/

#include "../../interface/KeyCodes.hpp"
#include "../Menu.hpp"

void Menu::processBack()
{
  if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
    if (currentState == MenuState::LOADING) {
      return;
    }
    if (currentState == MenuState::MAIN_MENU) {
      currentState = MenuState::LOADING;
      return;
    }
    currentState = MenuState::MAIN_MENU;
  }
}
