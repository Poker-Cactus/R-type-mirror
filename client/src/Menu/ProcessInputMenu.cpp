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
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
    if (currentState == MenuState::INTRO || currentState == MenuState::MAIN_MENU ||
        (currentState == MenuState::PROFILE && isProfileEditing())) {
      return;
    }
    currentState = MenuState::MAIN_MENU;
  }
}
