/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AIDifficultyMenu.cpp
*/

#include "AIDifficultyMenu.hpp"
#include "../../../include/Settings.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

AIDifficultyMenu::AIDifficultyMenu(std::shared_ptr<IRenderer> renderer, std::function<void(AIDifficulty)> onDifficultySelected)
    : m_renderer(renderer), font(nullptr), currentMenuIndex(0), m_onDifficultySelected(std::move(onDifficultySelected))
{
  // aiDifficultyItems is initialized in the header
}

AIDifficultyMenu::~AIDifficultyMenu()
{
  if (font != nullptr && m_renderer != nullptr) {
    m_renderer->freeFont(font);
  }
  if (clickedSound != nullptr && m_renderer != nullptr) {
    m_renderer->freeSound(clickedSound);
  }
  if (hoverSound != nullptr && m_renderer != nullptr) {
    m_renderer->freeSound(hoverSound);
  }
}

void AIDifficultyMenu::init()
{
  try {
    const int menuFontSize = 24;
    font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);
    clickedSound = m_renderer->loadSound("client/assets/audios/Retro3.mp3");
    hoverSound = m_renderer->loadSound("client/assets/Sounds/Hovering3.wav");
  } catch (const std::exception &e) {
    font = nullptr;
  }
}

void AIDifficultyMenu::render(int winWidth, int winHeight)
{
  if (font == nullptr) {
    return;
  }

  // Render title
  std::string title = "Select AI Difficulty";
  int titleWidth = 0;
  int titleHeight = 0;
  m_renderer->getTextSize(font, title, titleWidth, titleHeight);
  int titleX = (winWidth - titleWidth) / 2;
  int titleY = (winHeight / 2) - 120;
  m_renderer->drawText(font, title, titleX, titleY, Color{255, 255, 255, 255});

  // Render menu items
  for (size_t i = 0; i < aiDifficultyItems.size(); i++) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(font, aiDifficultyItems[i], textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 60;

    Color color = (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
    m_renderer->drawText(font, aiDifficultyItems[i], x, y, color);
  }
}

void AIDifficultyMenu::process(MenuState *currentState, Settings &settings)
{
  // Flèche bas - descendre dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    m_renderer->playSound(hoverSound);
    currentMenuIndex = (currentMenuIndex + 1) % aiDifficultyItems.size();
  }
  // Flèche haut - monter dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    m_renderer->playSound(hoverSound);
    currentMenuIndex = (currentMenuIndex - 1 + aiDifficultyItems.size()) % aiDifficultyItems.size();
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    m_renderer->playSound(clickedSound);
    std::string selectedButton = aiDifficultyItems[currentMenuIndex];
    std::cout << "[AIDifficultyMenu] Enter pressed on: " << selectedButton << '\n';
    if (selectedButton == "Weak") {
      settings.aiDifficulty = AIDifficulty::WEAK;
      std::cout << "[AIDifficultyMenu] AI difficulty set to WEAK" << '\n';
      if (m_onDifficultySelected) {
        m_onDifficultySelected(AIDifficulty::WEAK);
      }
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "Medium") {
      settings.aiDifficulty = AIDifficulty::MEDIUM;
      std::cout << "[AIDifficultyMenu] AI difficulty set to MEDIUM" << '\n';
      if (m_onDifficultySelected) {
        m_onDifficultySelected(AIDifficulty::MEDIUM);
      }
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "Strong") {
      settings.aiDifficulty = AIDifficulty::STRONG;
      std::cout << "[AIDifficultyMenu] AI difficulty set to STRONG" << '\n';
      if (m_onDifficultySelected) {
        m_onDifficultySelected(AIDifficulty::STRONG);
      }
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "No Ally") {
      settings.aiDifficulty = AIDifficulty::NO_ALLY;
      std::cout << "[AIDifficultyMenu] AI difficulty set to NO_ALLY" << '\n';
      if (m_onDifficultySelected) {
        m_onDifficultySelected(AIDifficulty::NO_ALLY);
      }
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "Back") {
      std::cout << "[AIDifficultyMenu] Going back to LOBBY" << '\n';
      *currentState = MenuState::LOBBY;
    }
  }
}
