/**
 * @file KeyToLabel.cpp
 * @brief Key code to human-readable label conversion implementation
 */

#include "../../include/KeyToLabel.hpp"
#include "../../interface/KeyCodes.hpp"

/**
 * @brief Convert key code to readable string label
 *
 * Maps keyboard key codes to their human-readable representations
 * for display in UI (e.g., settings menu, key binding screens).
 *
 * @param key Key code from KeyCodes enum
 * @return String label for the key (e.g., "Space", "Enter", "A")
 */
std::string KeyToLabel(int key)
{
  switch (key) {
  case KeyCode::KEY_UNKNOWN:
    return "Unknown";
  case KeyCode::KEY_RETURN:
    return "Enter";
  case KeyCode::KEY_ESCAPE:
    return "Esc";
  case KeyCode::KEY_BACKSPACE:
    return "Back";
  case KeyCode::KEY_TAB:
    return "Tab";
  case KeyCode::KEY_SPACE:
    return "Space";
  case KeyCode::KEY_DELETE:
    return "Del";

  case KeyCode::KEY_F11:
    return "F11";

  case KeyCode::KEY_UP:
    return "Up";
  case KeyCode::KEY_DOWN:
    return "Down";
  case KeyCode::KEY_LEFT:
    return "Left";
  case KeyCode::KEY_RIGHT:
    return "Right";

  case KeyCode::KEY_0:
    return "0";
  case KeyCode::KEY_1:
    return "1";
  case KeyCode::KEY_2:
    return "2";
  case KeyCode::KEY_3:
    return "3";
  case KeyCode::KEY_4:
    return "4";
  case KeyCode::KEY_5:
    return "5";
  case KeyCode::KEY_6:
    return "6";
  case KeyCode::KEY_7:
    return "7";
  case KeyCode::KEY_8:
    return "8";
  case KeyCode::KEY_9:
    return "9";

  case KeyCode::KEY_A:
    return "A";
  case KeyCode::KEY_B:
    return "B";
  case KeyCode::KEY_C:
    return "C";
  case KeyCode::KEY_D:
    return "D";
  case KeyCode::KEY_E:
    return "E";
  case KeyCode::KEY_F:
    return "F";
  case KeyCode::KEY_G:
    return "G";
  case KeyCode::KEY_H:
    return "H";
  case KeyCode::KEY_I:
    return "I";
  case KeyCode::KEY_J:
    return "J";
  case KeyCode::KEY_K:
    return "K";
  case KeyCode::KEY_L:
    return "L";
  case KeyCode::KEY_M:
    return "M";
  case KeyCode::KEY_N:
    return "N";
  case KeyCode::KEY_O:
    return "O";
  case KeyCode::KEY_P:
    return "P";
  case KeyCode::KEY_Q:
    return "Q";
  case KeyCode::KEY_R:
    return "R";
  case KeyCode::KEY_S:
    return "S";
  case KeyCode::KEY_T:
    return "T";
  case KeyCode::KEY_U:
    return "U";
  case KeyCode::KEY_V:
    return "V";
  case KeyCode::KEY_W:
    return "W";
  case KeyCode::KEY_X:
    return "X";
  case KeyCode::KEY_Y:
    return "Y";
  case KeyCode::KEY_Z:
    return "Z";
  default:
    return "";
  }
}