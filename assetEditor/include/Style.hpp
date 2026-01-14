/**
 * @file Style.hpp
 * @brief ImGui styling and font management
 *
 * Implements authentic macOS Dark Mode styling based on Apple HIG.
 */

#pragma once

#include <imgui.h>

namespace AssetEditor {

/**
 * @brief Apply macOS-inspired dark theme to ImGui
 *
 * Sets rounded corners, spacing, and macOS system colors
 * (Blue, Green, Orange, Red) with dark backgrounds.
 */
void ApplyMacOSStyle();

/**
 * @brief Load custom font with fallback support
 * @param io ImGui IO context
 *
 * Attempts to load bundled Inter font first, then falls back
 * to macOS system fonts, and finally to ImGui default font.
 */
void LoadFont(ImGuiIO& io);

} // namespace AssetEditor
