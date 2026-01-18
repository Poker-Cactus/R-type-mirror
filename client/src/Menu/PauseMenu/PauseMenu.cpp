/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PauseMenu implementation
*/

#include "PauseMenu.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

PauseMenu::PauseMenu(std::shared_ptr<IRenderer> renderer)
    : m_renderer(std::move(renderer))
{
  m_options = {"Resume", "Settings", "Quit"};
}

PauseMenu::~PauseMenu()
{
  if (m_font && m_renderer) {
    m_renderer->freeFont(m_font);
  }
  if (m_titleFont && m_renderer) {
    m_renderer->freeFont(m_titleFont);
  }
}

bool PauseMenu::init()
{
  if (!m_renderer) {
    return false;
  }

  try {
    // Use the project's main menu font for consistent style
    m_font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", 28);
    m_titleFont = m_renderer->loadFont("client/assets/font.opf/r-type.otf", 48);

    m_selectSound = m_renderer->loadSound("client/assets/Sounds/Hovering3.wav");
    m_confirmSound = m_renderer->loadSound("client/assets/audios/Retro3.mp3");
  } catch (const std::exception& e) {
    std::cerr << "[PauseMenu] Warning: Could not load resources: " << e.what() << std::endl;
  }

  return true;
}

void PauseMenu::render(int winWidth, int winHeight)
{
  // Dark overlay
  Color overlay = {.r = 0, .g = 0, .b = 0, .a = 180};
  m_renderer->drawRect(0, 0, winWidth, winHeight, overlay);

  // Title - "PAUSED"
  if (m_titleFont) {
    int titleW = 0, titleH = 0;
    m_renderer->getTextSize(m_titleFont, "PAUSED", titleW, titleH);
    int titleX = (winWidth - titleW) / 2;
    int titleY = winHeight / 4;
    
    Color titleColor = {.r = 255, .g = 255, .b = 255, .a = 255};
    m_renderer->drawText(m_titleFont, "PAUSED", titleX, titleY, titleColor);
  }

  // Menu options
  if (!m_font) return;

  const int optionHeight = 60;
  const int totalHeight = static_cast<int>(m_options.size()) * optionHeight;
  const int startY = (winHeight - totalHeight) / 2 + 50;

  for (size_t i = 0; i < m_options.size(); ++i) {
    bool selected = (static_cast<int>(i) == m_selectedIndex);
    
    int textW = 0, textH = 0;
    m_renderer->getTextSize(m_font, m_options[i], textW, textH);
    int textX = (winWidth - textW) / 2;
    int textY = startY + static_cast<int>(i) * optionHeight;

    // Selection indicator
    if (selected) {
      // Draw selection background (similar visual as main menus)
      Color bgColor = {.r = 5, .g = 10, .b = 25, .a = 160};
      int padding = 20;
      m_renderer->drawRect(textX - padding, textY - 5, textW + padding * 2, textH + 10, bgColor);
    }

    Color textColor = selected 
      ? Color{.r = 255, .g = 255, .b = 255, .a = 255}
      : Color{.r = 150, .g = 150, .b = 150, .a = 255};
    
    m_renderer->drawText(m_font, m_options[i], textX, textY, textColor);
  }

  // Help text
  Color helpColor = {.r = 180, .g = 180, .b = 180, .a = 200};
  const char* helpText = "Press ESC to resume";
  int helpW = 0, helpH = 0;
  m_renderer->getTextSize(m_font, helpText, helpW, helpH);
  m_renderer->drawText(m_font, helpText, (winWidth - helpW) / 2, winHeight - 80, helpColor);
}

PauseMenuAction PauseMenu::process()
{
  if (!m_renderer) {
    return PauseMenuAction::NONE;
  }

  // Navigate up
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP) || m_renderer->isKeyJustPressed(KeyCode::KEY_W)) {
    m_selectedIndex--;
    if (m_selectedIndex < 0) {
      m_selectedIndex = static_cast<int>(m_options.size()) - 1;
    }
    if (m_selectSound) {
      m_renderer->playSound(m_selectSound);
    }
  }

  // Navigate down
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN) || m_renderer->isKeyJustPressed(KeyCode::KEY_S)) {
    m_selectedIndex++;
    if (m_selectedIndex >= static_cast<int>(m_options.size())) {
      m_selectedIndex = 0;
    }
    if (m_selectSound) {
      m_renderer->playSound(m_selectSound);
    }
  }

  // ESC to resume
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE) || m_renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE)) {
    if (m_confirmSound) {
      m_renderer->playSound(m_confirmSound);
    }
    return PauseMenuAction::RESUME;
  }

  // Confirm selection
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN) || m_renderer->isKeyJustPressed(KeyCode::KEY_SPACE)) {
    if (m_confirmSound) {
      m_renderer->playSound(m_confirmSound);
    }
    
    switch (m_selectedIndex) {
      case 0:
        return PauseMenuAction::RESUME;
      case 1:
        return PauseMenuAction::SETTINGS;
      case 2:
        return PauseMenuAction::QUIT;
      default:
        return PauseMenuAction::NONE;
    }
  }

  return PauseMenuAction::NONE;
}

void PauseMenu::reset()
{
  m_selectedIndex = 0;
}
