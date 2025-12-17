#pragma once

// Generic key codes - independent of any library
// Each platform module (SDL, SFML, etc.) maps its native keycodes to these
namespace KeyCode
{
// Special keys
constexpr int KEY_UNKNOWN = 0;
constexpr int KEY_RETURN = 1;
constexpr int KEY_ESCAPE = 2;
constexpr int KEY_BACKSPACE = 3;
constexpr int KEY_TAB = 4;
constexpr int KEY_SPACE = 5;
constexpr int KEY_DELETE = 8;

// Function keys
constexpr int KEY_F11 = 9;

// Numbers (0-9)
constexpr int KEY_0 = 50;
constexpr int KEY_1 = 51;
constexpr int KEY_2 = 52;
constexpr int KEY_3 = 53;
constexpr int KEY_4 = 54;
constexpr int KEY_5 = 55;
constexpr int KEY_6 = 56;
constexpr int KEY_7 = 57;
constexpr int KEY_8 = 58;
constexpr int KEY_9 = 59;

// Arrow keys
constexpr int KEY_UP = 10;
constexpr int KEY_DOWN = 11;
constexpr int KEY_LEFT = 12;
constexpr int KEY_RIGHT = 13;

// Letters (A-Z)
constexpr int KEY_A = 20;
constexpr int KEY_B = 21;
constexpr int KEY_C = 22;
constexpr int KEY_D = 23;
constexpr int KEY_E = 24;
constexpr int KEY_F = 25;
constexpr int KEY_G = 26;
constexpr int KEY_H = 27;
constexpr int KEY_I = 28;
constexpr int KEY_J = 29;
constexpr int KEY_K = 30;
constexpr int KEY_L = 31;
constexpr int KEY_M = 32;
constexpr int KEY_N = 33;
constexpr int KEY_O = 34;
constexpr int KEY_P = 35;
constexpr int KEY_Q = 36;
constexpr int KEY_R = 37;
constexpr int KEY_S = 38;
constexpr int KEY_T = 39;
constexpr int KEY_U = 40;
constexpr int KEY_V = 41;
constexpr int KEY_W = 42;
constexpr int KEY_X = 43;
constexpr int KEY_Y = 44;
constexpr int KEY_Z = 45;
} // namespace KeyCode
