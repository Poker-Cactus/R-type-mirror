#pragma once
#include "Color.hpp"
#include <string>

class IText
{
public:
  virtual ~IText() = default;

  virtual void *loadFont(const std::string &filepath, int fontSize) = 0;
  virtual void freeFont(void *font) = 0;
  virtual void drawText(void *font, const std::string &text, int x, int y,
                        const Color &color = {255, 255, 255, 255}) = 0;
  virtual void getTextSize(void *font, const std::string &text, int &width, int &height) = 0;
};
