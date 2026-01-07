/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** MainMenu.cpp
*/

#include "MainMenu.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

MainMenu::MainMenu(std::shared_ptr<IRenderer> renderer)
    : m_renderer(renderer), font(nullptr), planet(nullptr), currentMenuIndex(0)
{
  mainMenuItems = {"Play", "Settings", "Profile", "Exit"};
}

MainMenu::~MainMenu()
{
  if (font != nullptr && m_renderer != nullptr) {
    m_renderer->freeFont(font);
  }
  if (planet != nullptr && m_renderer != nullptr) {
    m_renderer->freeTexture(planet);
  }
}

void MainMenu::init()
{
  try {
    const int menuFontSize = 24;
    planet = m_renderer->loadTexture("client/assets/moon-pack/moon1.png");
    font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);
  } catch (const std::exception &e) {
    planet = nullptr;
    font = nullptr;
  }
}

void MainMenu::render(int winWidth, int winHeight)
{
  if (font == nullptr) {
    return;
  }
  for (size_t i = 0; i < mainMenuItems.size(); i++) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(font, mainMenuItems[i], textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 90;

    Color color = (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
    m_renderer->drawText(font, mainMenuItems[i], x, y, color);
  }
}

void MainMenu::process(MenuState *currentState)
{
  // Flèche bas - descendre dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    currentMenuIndex = (currentMenuIndex + 1) % mainMenuItems.size();
  }
  // Flèche haut - monter dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    currentMenuIndex = (currentMenuIndex - 1 + mainMenuItems.size()) % mainMenuItems.size();
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    std::string selectedButton = mainMenuItems[currentMenuIndex];
    std::cout << "[MainMenu] Enter pressed on: " << selectedButton << '\n';
    if (selectedButton == "Play") {
      std::cout << "[MainMenu] Changing state to LOBBY" << '\n';
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
