/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** ProfileMenu.cpp
*/

#include "ProfileMenu.hpp"
#include "../../../interface/Color.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

void ProfileMenu::init(IRenderer *renderer, Settings &settings)
{
  this->settings = &settings;
  try {
    const int fontSize = 32;
    const int titleFontSize = 48;
    font = renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    titleFont = renderer->loadFont("client/assets/font.opf/r-type.otf", titleFontSize);
    currentUsername = settings.username;
  } catch (const std::exception &e) {
    std::cerr << "[ProfileMenu] Failed to initialize: " << e.what() << '\n';
  }
}

void ProfileMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
  if (font == nullptr || titleFont == nullptr) {
    return;
  }

  // Title
  int titleWidth = 0;
  int titleHeight = 0;
  renderer->getTextSize(titleFont, "Profile", titleWidth, titleHeight);
  int titleX = (winWidth - titleWidth) / 2;
  int titleY = winHeight / 4;
  renderer->drawText(titleFont, "Profile", titleX, titleY, Color{255, 255, 255, 255});

  // Username label
  int labelWidth = 0;
  int labelHeight = 0;
  renderer->getTextSize(font, "Username:", labelWidth, labelHeight);
  int labelX = (winWidth - labelWidth) / 2;
  int labelY = winHeight / 2 - 60;
  renderer->drawText(font, "Username:", labelX, labelY, Color{255, 255, 255, 255});

  // Username input box
  int boxWidth = 300;
  int boxHeight = 40;
  int boxX = (winWidth - boxWidth) / 2;
  int boxY = winHeight / 2 - 20;

  Color boxColor = (selectedOption == 0) ? Color{100, 100, 100, 255} : Color{50, 50, 50, 255};
  renderer->drawRect(boxX, boxY, boxWidth, boxHeight, boxColor);

  // Username text
  std::string displayUsername = currentUsername;
  if (isEditingUsername) {
    displayUsername += "_";
  }
  int usernameWidth = 0;
  int usernameHeight = 0;
  renderer->getTextSize(font, displayUsername, usernameWidth, usernameHeight);
  int usernameX = boxX + 10;
  int usernameY = boxY + (boxHeight - usernameHeight) / 2;
  Color textColor = isEditingUsername ? Color{255, 255, 0, 255} : Color{255, 255, 255, 255};
  renderer->drawText(font, displayUsername, usernameX, usernameY, textColor);

  // Save button
  int buttonWidth = 200;
  int buttonHeight = 40;
  int buttonX = (winWidth - buttonWidth) / 2;
  int buttonY = winHeight / 2 + 40;

  Color buttonColor = (selectedOption == 1) ? Color{0, 196, 199, 255} : Color{100, 100, 100, 255};
  renderer->drawRect(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);

  int saveWidth = 0;
  int saveHeight = 0;
  renderer->getTextSize(font, "Save", saveWidth, saveHeight);
  int saveX = buttonX + (buttonWidth - saveWidth) / 2;
  int saveY = buttonY + (buttonHeight - saveHeight) / 2;
  renderer->drawText(font, "Save", saveX, saveY, Color{255, 255, 255, 255});

  // Help text
  int helpY = winHeight - 60;
  renderer->drawText(font, "Use arrow keys to navigate, Enter to select/edit", (winWidth - 400) / 2, helpY, Color{255, 255, 255, 200});
}

void ProfileMenu::process(IRenderer *renderer)
{
  if (settings == nullptr) {
    return;
  }

  if (isEditingUsername) {
    // Handle text input
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      isEditingUsername = false;
    } else if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      if (!currentUsername.empty()) {
        currentUsername.pop_back();
      }
    } else {
      // Add letters (simplified - only uppercase for now)
      const int letterKeys[] = {
        KeyCode::KEY_A, KeyCode::KEY_B, KeyCode::KEY_C, KeyCode::KEY_D, KeyCode::KEY_E,
        KeyCode::KEY_F, KeyCode::KEY_G, KeyCode::KEY_H, KeyCode::KEY_I, KeyCode::KEY_J,
        KeyCode::KEY_K, KeyCode::KEY_L, KeyCode::KEY_M, KeyCode::KEY_N, KeyCode::KEY_O,
        KeyCode::KEY_P, KeyCode::KEY_Q, KeyCode::KEY_R, KeyCode::KEY_S, KeyCode::KEY_T,
        KeyCode::KEY_U, KeyCode::KEY_V, KeyCode::KEY_W, KeyCode::KEY_X, KeyCode::KEY_Y,
        KeyCode::KEY_Z, KeyCode::KEY_SPACE
      };

      const char letters[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', ' '
      };

      for (size_t i = 0; i < sizeof(letterKeys) / sizeof(letterKeys[0]); ++i) {
        if (renderer->isKeyJustPressed(letterKeys[i]) && currentUsername.length() < 20) {
          currentUsername += letters[i];
          break;
        }
      }
    }
  } else {
    // Handle menu navigation
    if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
      selectedOption = (selectedOption + 1) % 2;
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
      selectedOption = (selectedOption - 1 + 2) % 2;
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      if (selectedOption == 0) {
        // Start editing username
        isEditingUsername = true;
      } else if (selectedOption == 1) {
        // Save
        settings->username = currentUsername;
        std::cout << "[ProfileMenu] Username saved: " << currentUsername << '\n';
      }
    }
  }
}
