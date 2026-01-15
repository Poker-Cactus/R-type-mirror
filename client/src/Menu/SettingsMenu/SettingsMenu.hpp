/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SettingsMenu.hpp
*/
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
#include <memory>
#include <string>
#include <vector>

/**
 * @struct Component
 * @brief UI component with position and selection state
 */
struct Component {
  int rectX = 0; ///< X position
  int rectY = 0; ///< Y position
  int rectWidth = 0; ///< Width
  int rectHeight = 0; ///< Height
  bool isSelected = false; ///< Selection state
  std::string label; ///< Display label
};

/**
 * @enum SettingsCategory
 * @brief Settings menu categories
 */
enum class SettingsCategory : std::uint8_t {
  AUDIO = 0, ///< Audio settings
  GRAPHICS = 1, ///< Graphics settings
  CONTROLS = 2, ///< Control bindings
  DEBUG = 3 ///< Debug settings
};

/**
 * @enum SettingItemType
 * @brief Type of setting control
 */
enum class SettingItemType : std::uint8_t {
  SLIDER_INT, ///< Integer slider
  TOGGLE_BOOL, ///< Boolean toggle
  KEYBIND, ///< Key binding
  ENUM_CYCLE ///< Cycle through enum values
};

/**
 * @struct SettingItem
 * @brief Individual setting entry
 */
struct SettingItem {
  std::string label; ///< Setting label
  SettingItemType type = SettingItemType::SLIDER_INT; ///< Control type
  int minValue = 0; ///< Minimum value (for sliders)
  int maxValue = 100; ///< Maximum value (for sliders)
  int step = 5; ///< Step increment (for sliders)
  int *intTarget = nullptr; ///< Target integer variable
  bool *boolTarget = nullptr; ///< Target boolean variable
  std::vector<std::string> enumLabels; ///< Labels for enum values (for ENUM_CYCLE)
  std::uint8_t *enumTarget = nullptr; ///< Target enum variable (for ENUM_CYCLE)
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
  SettingsMenu(std::shared_ptr<IRenderer> renderer);
  ~SettingsMenu();

  /**
   * @brief Initialize settings menu
   * @param renderer Renderer interface
   */
  void init(Settings &settings);

  /**
   * @brief Render the settings menu
   * @param winWidth Window width
   * @param winHeight Window height
   */
  void render(int winWidth, int winHeight);

  /**
   * @brief Process user input
   */
  void process();

private:
  std::shared_ptr<IRenderer> m_renderer;
  Settings *settings = nullptr;
  void *font;
  void *titleFont;
  void *helpFont;

  // Catégories
  std::array<Component, 4> categoryTabs;
  SettingsCategory currentCategory;

  std::vector<SettingItem> audioItems;
  std::vector<SettingItem> graphicItems;
  std::vector<SettingItem> controlsItems;
  std::vector<SettingItem> debugItems;

  std::size_t selectedIndex = 0;
  bool isCapturingKey = false;
  bool isEditing = false;

  std::vector<SettingItem> &activeItems();
  std::string itemValueText(const SettingItem &item) const;
  void applyDelta(SettingItem &item, int direction);
  int captureKeyJustPressed() const;

  void renderRow(const Component &rowRect, const SettingItem &item, bool selected);
  void renderCategoryTab(const Component &tab, bool isActive);
  bool keyAlreadyInUse(int key);
  void *clickedSound;
  void *hoverSound;
  void *errorSound;
};
