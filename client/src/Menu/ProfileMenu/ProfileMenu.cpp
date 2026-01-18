/**
 * @file ProfileMenu.cpp
 * @brief Profile menu implementation.
 */

#include "ProfileMenu.hpp"
#include "../../../../common/include/Common.hpp"
#include "../../../interface/Color.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <cmath>
#include <iostream>

ProfileMenu::ProfileMenu(std::shared_ptr<IRenderer> renderer) : m_renderer(renderer) {}

ProfileMenu::~ProfileMenu()
{
  if (m_renderer) {
    if (font) {
      m_renderer->freeFont(font);
      font = nullptr;
    }
    if (titleFont) {
      m_renderer->freeFont(titleFont);
      titleFont = nullptr;
    }
  }
}

void ProfileMenu::init(Settings &settings)
{
  m_settings = &settings;
  try {
    const int fontSize = 32;
    const int titleFontSize = 48;
    font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    titleFont = m_renderer->loadFont("client/assets/font.opf/r-type.otf", titleFontSize);
    currentUsername = settings.username;
  } catch (const std::exception &e) {
    std::cerr << "[ProfileMenu] Failed to initialize: " << e.what() << '\n';
  }
}

void ProfileMenu::render(UNUSED int winWidth, UNUSED int winHeight)
{
  if (font == nullptr || titleFont == nullptr) {
    return;
  }

  // Title
  int titleWidth = 0;
  int titleHeight = 0;
  m_renderer->getTextSize(titleFont, "Profile", titleWidth, titleHeight);
  int titleX = (winWidth - titleWidth) / 2;
  int titleY = winHeight / 4;
  m_renderer->drawText(titleFont, "Profile", titleX, titleY, Color{255, 255, 255, 255});

  // Username label
  int labelWidth = 0;
  int labelHeight = 0;
  m_renderer->getTextSize(font, "Username", labelWidth, labelHeight);
  int labelX = (winWidth - labelWidth) / 2;
  int labelY = winHeight / 2 - 60;
  m_renderer->drawText(font, "Username", labelX, labelY, Color{255, 255, 255, 255});

  // Username input box
  int boxWidth = 300;
  int boxHeight = 40;
  int boxX = (winWidth - boxWidth) / 2;
  int boxY = winHeight / 2 - 20;

  Color boxColor = (selectedOption == 0) ? Color{100, 100, 100, 255} : Color{50, 50, 50, 255};
  m_renderer->drawRect(boxX, boxY, boxWidth, boxHeight, boxColor);

  // Username text
  std::string displayUsername = currentUsername;
  if (isEditingUsername) {
    displayUsername += "_";
  }
  int usernameWidth = 0;
  int usernameHeight = 0;
  m_renderer->getTextSize(font, displayUsername, usernameWidth, usernameHeight);
  int usernameX = boxX + 10;
  int usernameY = boxY + (boxHeight - usernameHeight) / 2;
  Color textColor = isEditingUsername ? Color{255, 255, 0, 255} : Color{255, 255, 255, 255};
  m_renderer->drawText(font, displayUsername, usernameX, usernameY, textColor);

  // Save button
  int buttonWidth = 200;
  int buttonHeight = 40;
  int buttonX = (winWidth - buttonWidth) / 2;
  int buttonY = winHeight / 2 + 40;

  Color buttonColor = (selectedOption == 1) ? Color{0, 196, 199, 255} : Color{100, 100, 100, 255};
  m_renderer->drawRect(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);

  int saveWidth = 0;
  int saveHeight = 0;
  m_renderer->getTextSize(font, "Save", saveWidth, saveHeight);
  int saveX = buttonX + (buttonWidth - saveWidth) / 2;
  int saveY = buttonY + (buttonHeight - saveHeight) / 2;
  m_renderer->drawText(font, "Save", saveX, saveY, Color{255, 255, 255, 255});

  // Help text
  int helpY = winHeight - 60;
  m_renderer->drawText(font, "Use arrow keys to navigate, Enter to select/edit", (winWidth - 400) / 2, helpY,
                       Color{255, 255, 255, 200});
}

void ProfileMenu::process(MenuState *currentState, Settings &settings)
{
  if (!isEditingUsername) {
    // Navigation
    if (m_renderer->isKeyJustPressed(settings.up)) {
      selectedOption = (selectedOption - 1 + 2) % 2;
    }
    if (m_renderer->isKeyJustPressed(settings.down)) {
      selectedOption = (selectedOption + 1) % 2;
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      if (selectedOption == 0) {
        isEditingUsername = true;
      } else if (selectedOption == 1) {
        // Save
        m_settings->username = currentUsername;
        *currentState = MenuState::MAIN_MENU;
      }
    }
  } else {
    // Editing username
    // Handle text input
    for (int key = KeyCode::KEY_A; key <= KeyCode::KEY_Z; ++key) {
      if (m_renderer->isKeyJustPressed(key) && currentUsername.length() < 8) {
        char c = 'a' + (key - KeyCode::KEY_A);
        currentUsername += c;
      }
    }
    for (int key = KeyCode::KEY_0; key <= KeyCode::KEY_9; ++key) {
      if (m_renderer->isKeyJustPressed(key) && currentUsername.length() < 8) {
        char c = '0' + (key - KeyCode::KEY_0);
        currentUsername += c;
      }
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_SPACE) && currentUsername.length() < 8) {
      currentUsername += ' ';
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE) && !currentUsername.empty()) {
      currentUsername.pop_back();
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      isEditingUsername = false;
    }
  }
}
