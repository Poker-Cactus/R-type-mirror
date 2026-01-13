/**
 * @file Style.cpp
 * @brief ImGui styling and font management implementation
 * 
 * Implements authentic macOS Dark Mode styling based on Apple HIG guidelines.
 * Colors are taken from official Apple Human Interface Guidelines (2024).
 */

#include "Style.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace AssetEditor {

/**
 * macOS System Colors (Dark Mode) - From Apple HIG
 * https://developer.apple.com/design/human-interface-guidelines/color#Specifications
 */
namespace MacColors {
    // Primary accent colors (Dark Mode values)
    constexpr ImVec4 Blue      = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    constexpr ImVec4 Green     = ImVec4(0.19f, 0.82f, 0.35f, 1.00f);  // RGB(48, 209, 88)
    constexpr ImVec4 Orange    = ImVec4(1.00f, 0.57f, 0.19f, 1.00f);  // RGB(255, 146, 48)
    constexpr ImVec4 Red       = ImVec4(1.00f, 0.26f, 0.27f, 1.00f);  // RGB(255, 66, 69)
    constexpr ImVec4 Yellow    = ImVec4(1.00f, 0.84f, 0.00f, 1.00f);  // RGB(255, 214, 0)
    constexpr ImVec4 Purple    = ImVec4(0.86f, 0.20f, 0.95f, 1.00f);  // RGB(219, 52, 242)
    constexpr ImVec4 Indigo    = ImVec4(0.43f, 0.49f, 1.00f, 1.00f);  // RGB(109, 124, 255)
    constexpr ImVec4 Teal      = ImVec4(0.00f, 0.82f, 0.88f, 1.00f);  // RGB(0, 210, 224)
    constexpr ImVec4 Cyan      = ImVec4(0.24f, 0.83f, 1.00f, 1.00f);  // RGB(60, 211, 254)
    
    // Gray scale (iOS/macOS Dark Mode)
    constexpr ImVec4 Gray6     = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);  // RGB(28, 28, 30)   - Darkest
    constexpr ImVec4 Gray5     = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);  // RGB(44, 44, 46)
    constexpr ImVec4 Gray4     = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);  // RGB(58, 58, 60)
    constexpr ImVec4 Gray3     = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);  // RGB(72, 72, 74)
    constexpr ImVec4 Gray2     = ImVec4(0.39f, 0.39f, 0.40f, 1.00f);  // RGB(99, 99, 102)
    constexpr ImVec4 Gray      = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);  // RGB(142, 142, 147)
    
    // Label colors (text hierarchy)
    constexpr ImVec4 Label           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // Primary text
    constexpr ImVec4 SecondaryLabel  = ImVec4(0.92f, 0.92f, 0.96f, 0.60f); // Secondary text
    constexpr ImVec4 TertiaryLabel   = ImVec4(0.92f, 0.92f, 0.96f, 0.30f); // Tertiary text
    constexpr ImVec4 QuaternaryLabel = ImVec4(0.92f, 0.92f, 0.96f, 0.16f); // Quaternary text
    
    // Separator
    constexpr ImVec4 Separator = ImVec4(0.33f, 0.33f, 0.35f, 0.60f);  // Subtle separator
}

void ApplyMacOSStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // ═══════════════════════════════════════════════════════════════════════
    // macOS Geometry - Rounded, spacious, modern
    // ═══════════════════════════════════════════════════════════════════════
    
    // macOS uses generous rounding (especially in Ventura+)
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
    
    // Subtle borders (macOS uses very subtle or no borders)
    style.WindowBorderSize  = 0.0f;  // macOS windows have no visible border
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;  // Inputs have no border in macOS
    style.PopupBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;
    
    // Separator thickness
    style.SeparatorTextBorderSize = 1.0f;
    
    // ═══════════════════════════════════════════════════════════════════════
    // macOS Dark Mode Colors
    // ═══════════════════════════════════════════════════════════════════════
    
    ImVec4* colors = style.Colors;
    
    // Text colors (hierarchy)
    colors[ImGuiCol_Text]                  = MacColors::Label;
    colors[ImGuiCol_TextDisabled]          = MacColors::TertiaryLabel;
    
    // Window backgrounds (layered depth)
    colors[ImGuiCol_WindowBg]              = MacColors::Gray6;           // Base window
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Transparent
    colors[ImGuiCol_PopupBg]               = ImVec4(0.15f, 0.15f, 0.17f, 0.95f); // Slightly elevated
    
    // Borders (very subtle in macOS)
    colors[ImGuiCol_Border]                = MacColors::Separator;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame backgrounds (inputs, sliders, etc.)
    colors[ImGuiCol_FrameBg]               = MacColors::Gray5;
    colors[ImGuiCol_FrameBgHovered]        = MacColors::Gray4;
    colors[ImGuiCol_FrameBgActive]         = MacColors::Gray3;
    
    // Title bar (darker in macOS Dark Mode)
    colors[ImGuiCol_TitleBg]               = MacColors::Gray6;
    colors[ImGuiCol_TitleBgActive]         = MacColors::Gray6;
    colors[ImGuiCol_TitleBgCollapsed]      = MacColors::Gray6;
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg]             = MacColors::Gray6;
    
    // Scrollbar (macOS style - subtle and thin)
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);  // Transparent
    colors[ImGuiCol_ScrollbarGrab]         = MacColors::Gray3;
    colors[ImGuiCol_ScrollbarGrabHovered]  = MacColors::Gray2;
    colors[ImGuiCol_ScrollbarGrabActive]   = MacColors::Gray;
    
    // Checkmark - System Green
    colors[ImGuiCol_CheckMark]             = MacColors::Green;
    
    // Slider - System Blue
    colors[ImGuiCol_SliderGrab]            = MacColors::Blue;
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.10f, 0.65f, 1.00f, 1.00f);
    
    // Buttons (macOS uses subtle gray buttons, accent color for primary actions)
    colors[ImGuiCol_Button]                = MacColors::Gray4;
    colors[ImGuiCol_ButtonHovered]         = MacColors::Gray3;
    colors[ImGuiCol_ButtonActive]          = MacColors::Blue;  // Active shows accent
    
    // Headers (CollapsibleHeader, TreeNode, Selectable when selected)
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
    colors[ImGuiCol_TabActive]             = MacColors::Gray4;
    colors[ImGuiCol_TabUnfocused]          = MacColors::Gray6;
    colors[ImGuiCol_TabUnfocusedActive]    = MacColors::Gray5;
    
    // Plot colors
    colors[ImGuiCol_PlotLines]             = MacColors::Cyan;
    colors[ImGuiCol_PlotLinesHovered]      = MacColors::Blue;
    colors[ImGuiCol_PlotHistogram]         = MacColors::Blue;
    colors[ImGuiCol_PlotHistogramHovered]  = MacColors::Cyan;
    
    // Tables
    colors[ImGuiCol_TableHeaderBg]         = MacColors::Gray5;
    colors[ImGuiCol_TableBorderStrong]     = MacColors::Separator;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(MacColors::Separator.x, MacColors::Separator.y, MacColors::Separator.z, 0.30f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
    
    // Text selection (accent color with transparency)
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(MacColors::Blue.x, MacColors::Blue.y, MacColors::Blue.z, 0.35f);
    
    // Drag and drop
    colors[ImGuiCol_DragDropTarget]        = MacColors::Blue;
    
    // Navigation highlight
    colors[ImGuiCol_NavHighlight]          = MacColors::Blue;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    
    // Modal dim
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

void LoadFont(ImGuiIO& io) {
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 3;
    fontConfig.PixelSnapH = true;
    
    // Try bundled Inter font first (cross-platform)
    const std::vector<std::string> bundledPaths = {
        "./assetEditor/vendor/fonts/Inter-Regular.otf",
        "../assetEditor/vendor/fonts/Inter-Regular.otf",
        "assetEditor/vendor/fonts/Inter-Regular.otf",
    };
    
    for (const auto& bundledFont : bundledPaths) {
        try {
            if (fs::exists(bundledFont)) {
                auto canonicalPath = fs::canonical(bundledFont);
                std::cout << "Trying to load Inter from: " << canonicalPath << std::endl;
                
                std::ifstream testFile(canonicalPath);
                if (!testFile.good()) {
                    std::cout << "Font file not readable, skipping..." << std::endl;
                    continue;
                }
                testFile.close();
                
                ImFont* font = io.Fonts->AddFontFromFileTTF(canonicalPath.string().c_str(), 15.0f, &fontConfig);
                if (font) {
                    std::cout << "Successfully loaded bundled Inter font!" << std::endl;
                    return;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error loading bundled font: " << e.what() << std::endl;
        }
    }
    
    // Fallback to system fonts (macOS only)
    const std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSText.ttf",
        "/Library/Fonts/SF-Pro-Text-Regular.otf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/HelveticaNeue.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };
    
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (fs::exists(path)) {
            ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), 15.0f, &fontConfig);
            if (font) {
                fontLoaded = true;
                std::cout << "Loaded system font: " << path << std::endl;
                break;
            }
        }
    }
    
    if (!fontLoaded) {
        ImFontConfig config;
        config.SizePixels = 15.0f;
        config.OversampleH = 3;
        config.OversampleV = 3;
        io.Fonts->AddFontDefault(&config);
        std::cout << "Using default ImGui font" << std::endl;
    }
}

} // namespace AssetEditor
