/**
 * @file Style.cpp
 * @brief ImGui styling and font management implementation
 *
 * Implements authentic macOS Dark Mode styling based on Apple HIG guidelines.
 * Colors are taken from official Apple Human Interface Guidelines (2024).
 */

#include "Style.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace AssetEditor {

// ═══════════════════════════════════════════════════════════════════════════
// macOS System Colors (Dark Mode) - From Apple HIG
// https://developer.apple.com/design/human-interface-guidelines/color
// ═══════════════════════════════════════════════════════════════════════════

namespace MacColors {
    // Primary accent colors (Dark Mode values)
    constexpr ImVec4 Blue      = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    constexpr ImVec4 Green     = ImVec4(0.19f, 0.82f, 0.35f, 1.00f);  // RGB(48, 209, 88)
    constexpr ImVec4 Orange    = ImVec4(1.00f, 0.57f, 0.19f, 1.00f);  // RGB(255, 146, 48)
    [[maybe_unused]] constexpr ImVec4 Red       = ImVec4(1.00f, 0.26f, 0.27f, 1.00f);  // RGB(255, 66, 69)
    [[maybe_unused]] constexpr ImVec4 Yellow    = ImVec4(1.00f, 0.84f, 0.00f, 1.00f);  // RGB(255, 214, 0)
    [[maybe_unused]] constexpr ImVec4 Purple    = ImVec4(0.86f, 0.20f, 0.95f, 1.00f);  // RGB(219, 52, 242)
    [[maybe_unused]] constexpr ImVec4 Indigo    = ImVec4(0.43f, 0.49f, 1.00f, 1.00f);  // RGB(109, 124, 255)
    [[maybe_unused]] constexpr ImVec4 Teal      = ImVec4(0.00f, 0.82f, 0.88f, 1.00f);  // RGB(0, 210, 224)
    [[maybe_unused]] constexpr ImVec4 Cyan      = ImVec4(0.24f, 0.83f, 1.00f, 1.00f);  // RGB(60, 211, 254)

    // Gray scale (iOS/macOS Dark Mode)
    constexpr ImVec4 Gray6     = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);  // RGB(28, 28, 30)
    constexpr ImVec4 Gray5     = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);  // RGB(44, 44, 46)
    constexpr ImVec4 Gray4     = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);  // RGB(58, 58, 60)
    constexpr ImVec4 Gray3     = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);  // RGB(72, 72, 74)
    constexpr ImVec4 Gray2     = ImVec4(0.39f, 0.39f, 0.40f, 1.00f);  // RGB(99, 99, 102)
    constexpr ImVec4 Gray      = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);  // RGB(142, 142, 147)

    // Label colors (text hierarchy)
    constexpr ImVec4 Label           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    [[maybe_unused]] constexpr ImVec4 SecondaryLabel  = ImVec4(0.92f, 0.92f, 0.96f, 0.60f);
    constexpr ImVec4 TertiaryLabel   = ImVec4(0.92f, 0.92f, 0.96f, 0.30f);
    [[maybe_unused]] constexpr ImVec4 QuaternaryLabel = ImVec4(0.92f, 0.92f, 0.96f, 0.16f);

    // Separator
    constexpr ImVec4 Separator = ImVec4(0.33f, 0.33f, 0.35f, 0.60f);
} // namespace MacColors

// ═══════════════════════════════════════════════════════════════════════════
// Style Application
// ═══════════════════════════════════════════════════════════════════════════

void ApplyMacOSStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // ─────────────────────────────────────────────────────────────────────────
    // macOS Geometry - Rounded, spacious, modern
    // ─────────────────────────────────────────────────────────────────────────

    style.WindowRounding    = 12.0f;
    style.ChildRounding     = 10.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;

    // macOS spacing - breathable and clean
    style.WindowPadding     = ImVec2(16, 16);
    style.FramePadding      = ImVec2(10, 6);
    style.ItemSpacing       = ImVec2(10, 8);
    style.ItemInnerSpacing  = ImVec2(8, 6);
    style.IndentSpacing     = 22.0f;
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;

    // Subtle borders
    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    style.SeparatorTextBorderSize = 1.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // macOS Dark Mode Colors
    // ─────────────────────────────────────────────────────────────────────────

    ImVec4* colors = style.Colors;

    // Text colors
    colors[ImGuiCol_Text]                  = MacColors::Label;
    colors[ImGuiCol_TextDisabled]          = MacColors::TertiaryLabel;

    // Window backgrounds
    colors[ImGuiCol_WindowBg]              = MacColors::Gray6;
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.15f, 0.15f, 0.17f, 0.95f);

    // Borders
    colors[ImGuiCol_Border]                = MacColors::Separator;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds
    colors[ImGuiCol_FrameBg]               = MacColors::Gray5;
    colors[ImGuiCol_FrameBgHovered]        = MacColors::Gray4;
    colors[ImGuiCol_FrameBgActive]         = MacColors::Gray3;

    // Title bar
    colors[ImGuiCol_TitleBg]               = MacColors::Gray6;
    colors[ImGuiCol_TitleBgActive]         = MacColors::Gray6;
    colors[ImGuiCol_TitleBgCollapsed]      = MacColors::Gray6;

    // Menu bar
    colors[ImGuiCol_MenuBarBg]             = MacColors::Gray6;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab]         = MacColors::Gray3;
    colors[ImGuiCol_ScrollbarGrabHovered]  = MacColors::Gray2;
    colors[ImGuiCol_ScrollbarGrabActive]   = MacColors::Gray;

    // Checkmark
    colors[ImGuiCol_CheckMark]             = MacColors::Green;

    // Slider
    colors[ImGuiCol_SliderGrab]            = MacColors::Blue;
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.10f, 0.65f, 1.00f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]                = MacColors::Gray4;
    colors[ImGuiCol_ButtonHovered]         = MacColors::Gray3;
    colors[ImGuiCol_ButtonActive]          = MacColors::Blue;

    // Headers
    colors[ImGuiCol_Header]                = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.30f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.50f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.70f);

    // Separator
    colors[ImGuiCol_Separator]             = MacColors::Separator;
    colors[ImGuiCol_SeparatorHovered]      = MacColors::Blue;
    colors[ImGuiCol_SeparatorActive]       = MacColors::Blue;

    // Resize grip
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered]     = MacColors::Gray2;
    colors[ImGuiCol_ResizeGripActive]      = MacColors::Blue;

    // Tabs
    colors[ImGuiCol_Tab]                   = MacColors::Gray5;
    colors[ImGuiCol_TabHovered]            = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.50f);
    colors[ImGuiCol_TabActive]             = MacColors::Blue;
    colors[ImGuiCol_TabUnfocused]          = MacColors::Gray5;
    colors[ImGuiCol_TabUnfocusedActive]    = MacColors::Gray4;

    // Docking (ces constantes n'existent pas dans toutes les versions d'ImGui)
    // colors[ImGuiCol_DockingPreview]        = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.40f);
    // colors[ImGuiCol_DockingEmptyBg]        = MacColors::Gray6;

    // Plot
    colors[ImGuiCol_PlotLines]             = MacColors::Blue;
    colors[ImGuiCol_PlotLinesHovered]      = MacColors::Orange;
    colors[ImGuiCol_PlotHistogram]         = MacColors::Green;
    colors[ImGuiCol_PlotHistogramHovered]  = MacColors::Orange;

    // Table
    colors[ImGuiCol_TableHeaderBg]         = MacColors::Gray5;
    colors[ImGuiCol_TableBorderStrong]     = MacColors::Gray4;
    colors[ImGuiCol_TableBorderLight]      = MacColors::Separator;
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);

    // Text selection
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.35f);

    // Drag/Drop
    colors[ImGuiCol_DragDropTarget]        = MacColors::Blue;

    // Nav
    colors[ImGuiCol_NavHighlight]          = MacColors::Blue;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    // Modal
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
}

// ═══════════════════════════════════════════════════════════════════════════
// Font Loading
// ═══════════════════════════════════════════════════════════════════════════

namespace {
    // Font search paths
    const std::vector<std::string> kFontSearchPaths = {
        "assetEditor/vendor/fonts/Inter-Regular.otf",
        "../assetEditor/vendor/fonts/Inter-Regular.otf",
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSText.ttf",
        "/Library/Fonts/SF-Pro.ttf",
    };

    constexpr float kFontSize = 15.0f;
} // namespace

void LoadFont(ImGuiIO& io) {
    // Try to find and load a font from search paths
    for (const auto& path : kFontSearchPaths) {
        if (fs::exists(path)) {
            std::cout << "[AssetEditor] Trying to load font from: \"" << path << "\"" << std::endl;
            ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), kFontSize);
            if (font) {
                std::cout << "[AssetEditor] Successfully loaded font!" << std::endl;
                return;
            }
        }
    }

    // Fallback to default ImGui font
    std::cout << "[AssetEditor] Using default ImGui font" << std::endl;
    io.Fonts->AddFontDefault();
}

} // namespace AssetEditor
