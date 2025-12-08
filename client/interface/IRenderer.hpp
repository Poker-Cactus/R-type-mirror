#pragma once

#include "IAudio.hpp"
#include "ICamera.hpp"
#include "ICollision.hpp"
#include "IInput.hpp"
#include "IRender.hpp"
#include "IShape.hpp"
#include "IText.hpp"
#include "ITexture.hpp"
#include "ITime.hpp"
#include "IWindow.hpp"

class IRenderer : public IRender,
                  public IWindow,
                  public IInput,
                  public ITexture,
                  public IText,
                  public IAudio,
                  public IShape,
                  public ICamera,
                  public ITime,
                  public ICollision
{
  public:
    virtual ~IRenderer() = default;
};
