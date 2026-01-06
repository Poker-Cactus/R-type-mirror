#pragma once
#include "../../../include/Settings.hpp"
#include "../../../interface/IRenderer.hpp"
#include "Settings.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct Component {
  int rectX = 0;
  int rectY = 0;
  int rectWidth = 0;
  int rectHeight = 0;
  bool isSelected = false;
  std::string label;
};

enum class SettingsCategory : std::uint8_t { AUDIO = 0, GRAPHICS = 1, CONTROLS = 2 };

enum class SettingItemType : std::uint8_t { SLIDER_INT, TOGGLE_BOOL, KEYBIND };

struct SettingItem {
  std::string label;
  SettingItemType type = SettingItemType::SLIDER_INT;
  int minValue = 0;
  int maxValue = 100;
  int step = 5;
  int *intTarget = nullptr;
  bool *boolTarget = nullptr;
};

class SettingsMenu
{
public:
  SettingsMenu() = default;
  ~SettingsMenu() {};
  void init(IRenderer *renderer, Settings &settings);
  void render(int winWidth, int winHeight, IRenderer *renderer);
  void process(IRenderer *renderer);

private:
  Settings *settings = nullptr;
  void *font;
  void *titleFont;
  void *helpFont;

  // Catégories
  std::array<Component, 3> categoryTabs;
  SettingsCategory currentCategory = SettingsCategory::AUDIO;

  std::vector<SettingItem> audioItems;
  std::vector<SettingItem> graphicItems;
  std::vector<SettingItem> controlsItems;

  std::size_t selectedIndex = 0;
  bool isCapturingKey = false;
  bool isEditing = false;

  std::vector<SettingItem> &activeItems();
  std::string itemValueText(const SettingItem &item) const;
  void applyDelta(SettingItem &item, int direction);
  int captureKeyJustPressed(IRenderer *renderer) const;

  void renderRow(IRenderer *renderer, const Component &rowRect, const SettingItem &item, bool selected);
  void renderCategoryTab(IRenderer *renderer, const Component &tab, bool isActive);
  bool keyAlreadyInUse(int key);
};