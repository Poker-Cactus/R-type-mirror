/**
 * @file SettingsMenu.hpp
 * @brief Settings configuration menu interface
 */

#pragma once
#include "../../../include/Settings.hpp"
#include "../../../interface/IRenderer.hpp"
#include "Settings.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @struct Component
 * @brief UI component with position and selection state
 */
struct Component {
  int rectX = 0;           ///< X position
  int rectY = 0;           ///< Y position
  int rectWidth = 0;       ///< Width
  int rectHeight = 0;      ///< Height
  bool isSelected = false; ///< Selection state
  std::string label;       ///< Display label
};

/**
 * @enum SettingsCategory
 * @brief Settings menu categories
 */
enum class SettingsCategory : std::uint8_t {
  AUDIO = 0,    ///< Audio settings
  GRAPHICS = 1, ///< Graphics settings
  CONTROLS = 2  ///< Control bindings
};

/**
 * @enum SettingItemType
 * @brief Type of setting control
 */
enum class SettingItemType : std::uint8_t {
  SLIDER_INT,  ///< Integer slider
  TOGGLE_BOOL, ///< Boolean toggle
  KEYBIND      ///< Key binding
};

/**
 * @struct SettingItem
 * @brief Individual setting entry
 */
struct SettingItem {
  std::string label;                      ///< Setting label
  SettingItemType type = SettingItemType::SLIDER_INT; ///< Control type
  int minValue = 0;                       ///< Minimum value (for sliders)
  int maxValue = 100;                     ///< Maximum value (for sliders)
  int step = 5;                           ///< Step increment (for sliders)
  int *intTarget = nullptr;               ///< Target integer variable
  bool *boolTarget = nullptr;             ///< Target boolean variable
};

/**
 * @class SettingsMenu
 * @brief Settings configuration interface
 *
 * Provides UI for configuring audio, graphics, and control settings.
 * Supports sliders, toggles, and key binding capture.
 */
class SettingsMenu
{
public:
  SettingsMenu() = default;
  ~SettingsMenu() {};

  /**
   * @brief Initialize settings menu
   * @param renderer Renderer interface
   * @param settings Game settings reference
   */
  void init(IRenderer *renderer, Settings &settings);

  /**
   * @brief Render the settings menu
   * @param winWidth Window width
   * @param winHeight Window height
   * @param renderer Renderer interface
   */
  void render(int winWidth, int winHeight, IRenderer *renderer);

  /**
   * @brief Process user input
   * @param renderer Renderer interface
   */
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