/**
 * @file IInput.hpp
 * @brief Input device abstraction (keyboard, mouse, gamepad).
 */

#pragma once

/**
 * @class IInput
 * @brief Interface for input polling.
 */
class IInput
{
public:
  virtual ~IInput() = default;

  /** @brief Check if a key is currently held down. */
  virtual bool isKeyPressed(int keycode) = 0;
  /** @brief Check if a key is currently released. */
  virtual bool isKeyReleased(int keycode) = 0;
  /** @brief Check if a key was pressed on this frame. */
  virtual bool isKeyJustPressed(int keycode) = 0;

  /** @brief Get current mouse cursor position. */
  virtual void getMousePosition(int &x, int &y) = 0;
  /** @brief Check if a mouse button is held down. */
  virtual bool isMouseButtonPressed(int button) = 0;

  /** @brief Get the number of connected gamepads. */
  virtual int getNumGamepads() = 0;
  /** @brief Check if a gamepad button is pressed. */
  virtual bool isGamepadButtonPressed(int gamepadIndex, int button) = 0;
  /** @brief Get a gamepad axis value. */
  virtual float getGamepadAxis(int gamepadIndex, int axis) = 0;
};
