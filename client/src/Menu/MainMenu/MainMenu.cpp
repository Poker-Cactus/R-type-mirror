/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** MainMenu.cpp
*/

#include "MainMenu.hpp"
#include "../../../interface/KeyCodes.hpp"

void MainMenu::init(IRenderer *renderer)
{
  try {
    const int menuFontSize = 24;
    planet = renderer->loadTexture("client/assets/moon-pack/moon1.png");
    font = renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);

  } catch (const std::exception &e) {
    planet = nullptr;
  }
}

void MainMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
  if (font == nullptr) {
    return;
  }

  for (size_t i = 0; i < mainMenuItems.size(); i++) {
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(font, mainMenuItems[i], textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 90;

    Color color = (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
    renderer->drawText(font, mainMenuItems[i], x, y, color);
  }
}

void MainMenu::process(MenuState *currentState, IRenderer *renderer)
{
  // Flèche bas - descendre dans le menu
  if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    currentMenuIndex = (currentMenuIndex + 1) % mainMenuItems.size();
  }
  // Flèche haut - monter dans le menu
  if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    currentMenuIndex = (currentMenuIndex - 1 + mainMenuItems.size()) % mainMenuItems.size();
  }
  if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    std::string selectedButton = mainMenuItems[currentMenuIndex];
    if (selectedButton == "Play") {
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "Settings") {
      *currentState = MenuState::SETTINGS;
    } else if (selectedButton == "Profile") {
      *currentState = MenuState::PROFILE;
    } else {
      *currentState = MenuState::EXIT;
    }
  }
}