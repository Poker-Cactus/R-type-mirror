/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** LobbyMenu.cpp
*/

#include "SettingsMenu.hpp"
#include "../../../include/KeyToLabel.hpp"
#include "../../../include/Settings.hpp"
#include "../interface/Color.hpp"
#include "../interface/KeyCodes.hpp"
#include <cmath>

static int clampInt(int value, int minValue, int maxValue)
{
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

void SettingsMenu::init(IRenderer *renderer, Settings &settings)
{
  this->settings = &settings;
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
    const int tabHeight = static_cast<int>(winHeight * 0.06);
    const int tabY = static_cast<int>(winHeight * 0.05);

    for (size_t i = 0; i < categoryTabs.size(); i++) {
      categoryTabs[i].rectX =
        (winWidth / 2) - (static_cast<int>(categoryLabels.size()) * tabWidth / 2) + (static_cast<int>(i) * tabWidth);
      categoryTabs[i].rectY = tabY;
      categoryTabs[i].rectWidth = tabWidth;
      categoryTabs[i].rectHeight = tabHeight;
      categoryTabs[i].label = categoryLabels[i];
      categoryTabs[i].isSelected = (i == 0);
    }

    audioItems.clear();
    audioItems.push_back({.label = "Master Volume",
                          .type = SettingItemType::SLIDER_INT,
                          .minValue = 0,
                          .maxValue = 100,
                          .step = 5,
                          .intTarget = &this->settings->masterVolume});
    audioItems.push_back({.label = "Music Volume",
                          .type = SettingItemType::SLIDER_INT,
                          .minValue = 0,
                          .maxValue = 100,
                          .step = 5,
                          .intTarget = &this->settings->musicVolume});
    audioItems.push_back({.label = "SFX Volume",
                          .type = SettingItemType::SLIDER_INT,
                          .minValue = 0,
                          .maxValue = 100,
                          .step = 5,
                          .intTarget = &this->settings->sfxVolume});

    graphicItems.clear();
    graphicItems.push_back(
      {.label = "Fullscreen", .type = SettingItemType::TOGGLE_BOOL, .boolTarget = &this->settings->fullScreen});

    controlsItems.clear();
    controlsItems.push_back({.label = "Move Up", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->up});
    controlsItems.push_back(
      {.label = "Move Down", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->down});
    controlsItems.push_back(
      {.label = "Move Left", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->left});
    controlsItems.push_back(
      {.label = "Move Right", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->right});
    controlsItems.push_back({.label = "Shoot", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->shoot});
    controlsItems.push_back(
      {.label = "Charged Shoot", .type = SettingItemType::KEYBIND, .intTarget = &this->settings->chargedShoot});

    selectedIndex = 0;
    isCapturingKey = false;
    isEditing = false;
  } catch (const std::exception &e) {
    (void)e;
  }
}

std::vector<SettingItem> &SettingsMenu::activeItems()
{
  switch (currentCategory) {
  case SettingsCategory::AUDIO:
    return audioItems;
  case SettingsCategory::GRAPHICS:
    return graphicItems;
  case SettingsCategory::CONTROLS:
    return controlsItems;
  }
  return audioItems;
}

std::string SettingsMenu::itemValueText(const SettingItem &item) const
{
  if (isCapturingKey && item.type == SettingItemType::KEYBIND) {
    return "Press A Key";
  }

  switch (item.type) {
  case SettingItemType::SLIDER_INT: {
    const int value = (item.intTarget != nullptr) ? *item.intTarget : 0;
    return std::to_string(value) + "%";
  }
  case SettingItemType::TOGGLE_BOOL: {
    const bool value = (item.boolTarget != nullptr) ? *item.boolTarget : false;
    return value ? "On" : "Off";
  }
  case SettingItemType::KEYBIND: {
    const int key = (item.intTarget != nullptr) ? *item.intTarget : KeyCode::KEY_UNKNOWN;
    std::string label = KeyToLabel(key);
    if (label.empty())
      label = "?";
    return label;
  }
  }
  return "";
}

void SettingsMenu::applyDelta(SettingItem &item, int direction)
{
  if (direction == 0)
    return;

  switch (item.type) {
  case SettingItemType::SLIDER_INT:
    if (item.intTarget != nullptr) {
      const int delta = (direction > 0) ? item.step : -item.step;
      *item.intTarget = clampInt(*item.intTarget + delta, item.minValue, item.maxValue);
    }
    break;
  case SettingItemType::TOGGLE_BOOL:
    if (item.boolTarget != nullptr) {
      *item.boolTarget = !*item.boolTarget;
    }
    break;
  case SettingItemType::KEYBIND:
    break;
  }
}

int SettingsMenu::captureKeyJustPressed(IRenderer *renderer) const
{
  const int candidates[] = {
    KeyCode::KEY_RETURN, KeyCode::KEY_ESCAPE, KeyCode::KEY_BACKSPACE, KeyCode::KEY_TAB,  KeyCode::KEY_SPACE,
    KeyCode::KEY_DELETE, KeyCode::KEY_F11,    KeyCode::KEY_UP,        KeyCode::KEY_DOWN, KeyCode::KEY_LEFT,
    KeyCode::KEY_RIGHT,  KeyCode::KEY_0,      KeyCode::KEY_1,         KeyCode::KEY_2,    KeyCode::KEY_3,
    KeyCode::KEY_4,      KeyCode::KEY_5,      KeyCode::KEY_6,         KeyCode::KEY_7,    KeyCode::KEY_8,
    KeyCode::KEY_9,      KeyCode::KEY_A,      KeyCode::KEY_B,         KeyCode::KEY_C,    KeyCode::KEY_D,
    KeyCode::KEY_E,      KeyCode::KEY_F,      KeyCode::KEY_G,         KeyCode::KEY_H,    KeyCode::KEY_I,
    KeyCode::KEY_J,      KeyCode::KEY_K,      KeyCode::KEY_L,         KeyCode::KEY_M,    KeyCode::KEY_N,
    KeyCode::KEY_O,      KeyCode::KEY_P,      KeyCode::KEY_Q,         KeyCode::KEY_R,    KeyCode::KEY_S,
    KeyCode::KEY_T,      KeyCode::KEY_U,      KeyCode::KEY_V,         KeyCode::KEY_W,    KeyCode::KEY_X,
    KeyCode::KEY_Y,      KeyCode::KEY_Z,
  };

  for (int key : candidates) {
    if (renderer->isKeyJustPressed(key)) {
      return key;
    }
  }
  return KeyCode::KEY_UNKNOWN;
}

void SettingsMenu::renderCategoryTab(IRenderer *renderer, const Component &tab, bool isActive)
{
  Color textColor =
    isActive ? Color{.r = 255, .g = 255, .b = 255, .a = 255} : Color{.r = 150, .g = 150, .b = 150, .a = 120};

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

void SettingsMenu::renderRow(IRenderer *renderer, const Component &rowRect, const SettingItem &item, bool selected)
{
  const int borderThickness = 6;

  if (selected) {
    Color bgColor = {.r = 5, .g = 10, .b = 25, .a = 120};
    Color border = {.r = 180, .g = 180, .b = 180, .a = 255};

    renderer->drawRect(rowRect.rectX, rowRect.rectY, rowRect.rectWidth, rowRect.rectHeight, bgColor);

    for (int i = 0; i < borderThickness; i++) {
      border.a = 255 - ((borderThickness - 1 - i) * 40);
      renderer->drawRectOutline(rowRect.rectX + i, rowRect.rectY + i, rowRect.rectWidth - (i * 2),
                                rowRect.rectHeight - (i * 2), border);
    }
  }

  int textWidth = 0;
  int textHeight = 0;
  renderer->getTextSize(font, item.label, textWidth, textHeight);
  int textX = rowRect.rectX + 10;
  int textY = rowRect.rectY + (rowRect.rectHeight - textHeight) / 2;

  Color textColor =
    selected ? Color{.r = 255, .g = 255, .b = 255, .a = 255} : Color{.r = 180, .g = 180, .b = 180, .a = 255};
  renderer->drawText(font, item.label, textX, textY, textColor);

  const std::string value = itemValueText(item);
  if (!value.empty()) {
    std::string displayValue = value;
    if (selected && isEditing && !isCapturingKey) {
      displayValue = "> " + displayValue + " <";
    }

    int valueW = 0;
    int valueH = 0;
    renderer->getTextSize(font, displayValue, valueW, valueH);
    const int valueX = rowRect.rectX + rowRect.rectWidth - valueW - 10;
    const int valueY = rowRect.rectY + (rowRect.rectHeight - valueH) / 2;
    renderer->drawText(font, displayValue, valueX, valueY, textColor);
  }
}

void SettingsMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
  const Color darkOverlay = {.r = 0, .g = 0, .b = 0, .a = 120};
  renderer->drawRect(0, 0, winWidth, winHeight, darkOverlay);

  for (size_t i = 0; i < categoryTabs.size(); i++) {
    bool isActive = (static_cast<size_t>(currentCategory) == i);
    renderCategoryTab(renderer, categoryTabs[i], isActive);
  }

  const auto &items = activeItems();
  const int rowWidth = winWidth * 0.5;
  const int rowHeight = winHeight * 0.07;
  const int startX = (winWidth - rowWidth) / 2;
  const int totalHeight = static_cast<int>(items.size()) * rowHeight;
  const int startY = (winHeight - totalHeight) / 2;

  for (std::size_t i = 0; i < items.size(); ++i) {
    Component row;
    row.rectX = startX;
    row.rectY = startY + static_cast<int>(i) * rowHeight;
    row.rectWidth = rowWidth;
    row.rectHeight = rowHeight;
    const bool selected = (i == selectedIndex);
    renderRow(renderer, row, items[i], selected);
  }

  const Color helpTextColor = {.r = 255, .g = 255, .b = 255, .a = 200};
  const int helpTextX = 60;
  const int helpTextY = winHeight - 60;
  renderer->drawText(helpFont, "Press return to get back", helpTextX, helpTextY, helpTextColor);
}

bool SettingsMenu::keyAlreadyInUse(int key)
{
  for (auto &item : activeItems()) {
    if (*item.intTarget == key) {
      return false;
    }
  }
  return true;
}

void SettingsMenu::process(IRenderer *renderer)
{
  if (settings == nullptr) {
    return;
  }

  if (isCapturingKey) {
    if (renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE) || renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      isCapturingKey = false;
      isEditing = false;
      return;
    }

    int key = captureKeyJustPressed(renderer);
    if (key != KeyCode::KEY_UNKNOWN) {
      auto &items = activeItems();
      if (!items.empty() && selectedIndex < items.size()) {
        auto &item = items[selectedIndex];
        if (item.type == SettingItemType::KEYBIND && item.intTarget != nullptr) {
          if (keyAlreadyInUse(key)) {
            *item.intTarget = key;
          }
        }
      }
      isCapturingKey = false;
      isEditing = false;
    }
    return;
  }

  if (renderer->isKeyJustPressed(KeyCode::KEY_ESCAPE)) {
    isEditing = false;
    return;
  }

  if (renderer->isKeyJustPressed(KeyCode::KEY_LEFT) && !isEditing) {
    int catIndex = static_cast<int>(currentCategory);
    if (catIndex > 0) {
      currentCategory = static_cast<SettingsCategory>(catIndex - 1);
      selectedIndex = 0;
    }
  }
  if (renderer->isKeyJustPressed(KeyCode::KEY_RIGHT) && !isEditing) {
    int catIndex = static_cast<int>(currentCategory);
    if (catIndex < 2) {
      currentCategory = static_cast<SettingsCategory>(catIndex + 1);
      selectedIndex = 0;
    }
  }

  auto &items = activeItems();
  if (items.empty()) {
    return;
  }

  if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    if (selectedIndex + 1 < items.size()) {
      selectedIndex++;
      isEditing = false;
    }
  }
  if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    if (selectedIndex > 0) {
      selectedIndex--;
      isEditing = false;
    }
  }

  if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    auto &item = items[selectedIndex];
    if (item.type == SettingItemType::KEYBIND) {
      isEditing = true;
      isCapturingKey = true;
      return;
    }
    if (item.type == SettingItemType::TOGGLE_BOOL) {
      applyDelta(item, +1);
      return;
    }
    isEditing = !isEditing;
    return;
  }

  // Only modify values while in edit mode
  if (isEditing) {
    if (renderer->isKeyJustPressed(KeyCode::KEY_LEFT)) {
      applyDelta(items[selectedIndex], -1);
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_RIGHT)) {
      applyDelta(items[selectedIndex], +1);
    }
  }
}
