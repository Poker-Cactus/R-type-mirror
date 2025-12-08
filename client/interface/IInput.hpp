#pragma once

class IInput
{
  public:
    virtual ~IInput() = default;

    // Keyboard
    virtual bool isKeyPressed(int keycode) = 0;
    virtual bool isKeyReleased(int keycode) = 0;
    virtual bool isKeyJustPressed(int keycode) = 0;

    // Mouse
    virtual void getMousePosition(int &x, int &y) = 0;
    virtual bool isMouseButtonPressed(int button) = 0;

    // Gamepad
    virtual int getNumGamepads() = 0;
    virtual bool isGamepadButtonPressed(int gamepadIndex, int button) = 0;
    virtual float getGamepadAxis(int gamepadIndex, int axis) = 0;
};
