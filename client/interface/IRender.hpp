#pragma once
#include "Color.hpp"

class IRender
{
  public:
    virtual ~IRender() = default;

    virtual void clear(const Color &color = {0, 0, 0, 255}) = 0;
    virtual void present() = 0;
};