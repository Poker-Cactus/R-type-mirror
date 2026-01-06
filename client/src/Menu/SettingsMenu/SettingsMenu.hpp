#pragma once
#include "../../../interface/IRenderer.hpp"
#include <array>
#include <string>
#include <vector>

struct Component {
  int rectX = 0;
  int rectY = 0;
  int rectWidth = 0;
  int rectHeight = 0;
  bool isSelected = false;
  std::string label = "";
};

enum class SettingsCategory { AUDIO = 0, GRAPHICS = 1, CONTROLS = 2 };

class SettingsMenu
{
public:
  SettingsMenu() = default;
  ~SettingsMenu() {}
  void init(IRenderer *renderer);
  void render(int winWidth, int winHeight, IRenderer *renderer);
  void process(IRenderer *renderer);

private:
  void *font;
  void *titleFont;
  void *helpFont;

  // Catégories
  std::array<Component, 3> categoryTabs;
  SettingsCategory currentCategory = SettingsCategory::AUDIO;

  // Boutons par catégorie
  std::array<Component, 5> audioButtons;
  std::array<Component, 5> graphicButtons;
  std::array<Component, 5> controlsButtons;

  template <size_t N>
  void initButtons(std::array<Component, N> &buttons, const std::vector<std::string> &labels, int winWidth,
                   int winHeight, int startY);
  void renderButton(IRenderer *renderer, const Component &button);
  void renderCategoryTab(IRenderer *renderer, const Component &tab, bool isActive);
};