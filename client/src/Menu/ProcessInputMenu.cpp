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
    // Don't process back button if profile menu is editing
    if (currentState == MenuState::PROFILE && isProfileEditing()) {
      return;
    }
    if (currentState == MenuState::INTRO || currentState == MenuState::MAIN_MENU) {
      return;
    }
    currentState = MenuState::MAIN_MENU;
  }
}
