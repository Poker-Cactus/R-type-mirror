/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** LobbyMenu.cpp
*/

#include "SettingsMenu.hpp"
#include "../interface/Color.hpp"
#include "../interface/KeyCodes.hpp"
#include <cmath>

void SettingsMenu::init(IRenderer *renderer)
{
  try {
    const int fontSize = 32;
    const int titleFontSize = 48;
    const int helpFontSize = 18;
    font = renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    titleFont = renderer->loadFont("client/assets/font.opf/r-type.otf", titleFontSize);
    helpFont = renderer->loadFont("client/assets/font.opf/r-type.otf", helpFontSize);

    int winWidth = renderer->getWindowWidth();
    int winHeight = renderer->getWindowHeight();

    std::vector<std::string> categoryLabels = {"Audio", "Graphics", "Controls"};
    const int tabWidth = winWidth / 4;
    const int tabHeight = winHeight * 0.06;
    const int tabY = winHeight * 0.05;

    for (size_t i = 0; i < categoryTabs.size(); i++) {
      categoryTabs[i].rectX = (winWidth / 2) - (categoryLabels.size() * tabWidth / 2) + (i * tabWidth);
      categoryTabs[i].rectY = tabY;
      categoryTabs[i].rectWidth = tabWidth;
      categoryTabs[i].rectHeight = tabHeight;
      categoryTabs[i].label = categoryLabels[i];
      categoryTabs[i].isSelected = (i == 0);
    }

    const int buttonHeight = winHeight * 0.07;
    const int numButtons = 5;
    const int totalButtonsHeight = numButtons * buttonHeight;
    const int buttonsStartY = (winHeight - totalButtonsHeight) / 2;

    std::vector<std::string> audioLabels = {"Master Volume", "Music Volume", "SFX Volume", "Voice Volume"};
    initButtons(audioButtons, audioLabels, winWidth, winHeight, buttonsStartY);

    std::vector<std::string> graphicLabels = {"Fullscreen"};
    initButtons(graphicButtons, graphicLabels, winWidth, winHeight, buttonsStartY);

    std::vector<std::string> controlsLabels = {"Move Up", "Move Down", "Move Left", "Move Right"};
    initButtons(controlsButtons, controlsLabels, winWidth, winHeight, buttonsStartY);
  } catch (const std::exception &e) {
  }
}

template <size_t N>
void SettingsMenu::initButtons(std::array<Component, N> &buttons, const std::vector<std::string> &labels, int winWidth,
                               int winHeight, int startY)
{
  const int buttonWidth = winWidth * 0.5;
  const int buttonHeight = winHeight * 0.07;
  const int startX = (winWidth - buttonWidth) / 2;

  size_t maxButtons = std::min(buttons.size(), labels.size());

  for (size_t i = 0; i < maxButtons; i++) {
    buttons[i].rectX = startX;
    buttons[i].rectY = startY + (i * buttonHeight);
    buttons[i].rectWidth = buttonWidth;
    buttons[i].rectHeight = buttonHeight;
    buttons[i].label = labels[i];
    buttons[i].isSelected = (i == 0);
  }
}

void SettingsMenu::renderCategoryTab(IRenderer *renderer, const Component &tab, bool isActive)
{
  // Couleur du texte avec moins d'opacité pour les non-actifs
  Color textColor =
    isActive ? Color{.r = 255, .g = 255, .b = 255, .a = 255} : Color{.r = 150, .g = 150, .b = 150, .a = 120};

  // Centrer le texte avec la grosse police
  int textWidth = 0;
  int textHeight = 0;
  renderer->getTextSize(titleFont, tab.label, textWidth, textHeight);
  int textX = tab.rectX + (tab.rectWidth - textWidth) / 2;
  int textY = tab.rectY + (tab.rectHeight - textHeight) / 2;

  renderer->drawText(titleFont, tab.label, textX, textY, textColor);

  if (isActive) {
    Color underlineColor = {.r = 255, .g = 255, .b = 255, .a = 255};
    int underlineWidth = textWidth * 0.6;
    int underlineX = textX + (textWidth - underlineWidth) / 2;
    int underlineY = textY + textHeight + 8;
    int underlineThickness = 4;

    for (int i = 0; i < underlineThickness; i++) {
      renderer->drawLine(underlineX, underlineY + i, underlineX + underlineWidth, underlineY + i, underlineColor);
    }
  }
}

void SettingsMenu::renderButton(IRenderer *renderer, const Component &button)
{
  const int borderThickness = 6;

  if (button.isSelected) {
    Color bgColor = {.r = 5, .g = 10, .b = 25, .a = 120};
    Color border = {.r = 180, .g = 180, .b = 180, .a = 255};

    renderer->drawRect(button.rectX, button.rectY, button.rectWidth, button.rectHeight, bgColor);

    for (int i = 0; i < borderThickness; i++) {
      border.a = 255 - ((borderThickness - 1 - i) * 40);
      renderer->drawRectOutline(button.rectX + i, button.rectY + i, button.rectWidth - (i * 2),
                                button.rectHeight - (i * 2), border);
    }
  }

  int textWidth = 0;
  int textHeight = 0;
  renderer->getTextSize(font, button.label, textWidth, textHeight);
  int textX = button.rectX + 10; // Petit padding à gauche
  int textY = button.rectY + (button.rectHeight - textHeight) / 2;

  Color textColor =
    button.isSelected ? Color{.r = 255, .g = 255, .b = 255, .a = 255} : Color{.r = 180, .g = 180, .b = 180, .a = 255};
  renderer->drawText(font, button.label, textX, textY, textColor);
}

void SettingsMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
  const Color darkOverlay = {.r = 0, .g = 0, .b = 0, .a = 120};
  renderer->drawRect(0, 0, winWidth, winHeight, darkOverlay);

  for (size_t i = 0; i < categoryTabs.size(); i++) {
    bool isActive = (static_cast<size_t>(currentCategory) == i);
    renderCategoryTab(renderer, categoryTabs[i], isActive);
  }

  switch (currentCategory) {
  case SettingsCategory::AUDIO:
    for (const auto &button : audioButtons) {
      renderButton(renderer, button);
    }
    break;
  case SettingsCategory::GRAPHICS:
    for (const auto &button : graphicButtons) {
      renderButton(renderer, button);
    }
    break;
  case SettingsCategory::CONTROLS:
    for (const auto &button : controlsButtons) {
      renderButton(renderer, button);
    }
    break;
  }

  const Color helpTextColor = {.r = 255, .g = 255, .b = 255, .a = 200};
  const int helpTextX = 60;
  const int helpTextY = winHeight - 60; // Plus de marge en bas
  renderer->drawText(helpFont, "Press return to get back", helpTextX, helpTextY, helpTextColor);
}

void SettingsMenu::process(IRenderer *renderer)
{
  if (renderer->isKeyJustPressed(KeyCode::KEY_LEFT)) {
    int catIndex = static_cast<int>(currentCategory);
    if (catIndex > 0) {
      currentCategory = static_cast<SettingsCategory>(catIndex - 1);
    }
  }
  if (renderer->isKeyJustPressed(KeyCode::KEY_RIGHT)) {
    int catIndex = static_cast<int>(currentCategory);
    if (catIndex < 2) {
      currentCategory = static_cast<SettingsCategory>(catIndex + 1);
    }
  }

  std::array<Component, 4> *activeButtons = nullptr;
  switch (currentCategory) {
  case SettingsCategory::AUDIO:
    activeButtons = &audioButtons;
    break;
  case SettingsCategory::GRAPHICS:
    activeButtons = &graphicButtons;
    break;
  case SettingsCategory::CONTROLS:
    activeButtons = &controlsButtons;
    break;
  }

  if (activeButtons != nullptr) {
    if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
      for (size_t i = 0; i < activeButtons->size(); i++) {
        if ((*activeButtons)[i].isSelected && i < activeButtons->size() - 1) {
          (*activeButtons)[i].isSelected = false;
          (*activeButtons)[i + 1].isSelected = true;
          break;
        }
      }
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
      for (size_t i = 0; i < activeButtons->size(); i++) {
        if ((*activeButtons)[i].isSelected && i > 0) {
          (*activeButtons)[i].isSelected = false;
          (*activeButtons)[i - 1].isSelected = true;
          break;
        }
      }
    }
  }
}
