/**
 * @file MainMenu.cpp
 * @brief Main menu interface implementation
 *
 * Uses authentic macOS system colors from Apple HIG.
 */

#include "MainMenu.hpp"
#include "EditorState.hpp"
#include "SpriteEditor.hpp"

#include <imgui.h>
#include <SDL.h>

namespace AssetEditor {

// ═══════════════════════════════════════════════════════════════════════════
// macOS System Colors (Dark Mode)
// ═══════════════════════════════════════════════════════════════════════════

namespace Colors {
    constexpr ImVec4 Blue  = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    constexpr ImVec4 Green = ImVec4(0.19f, 0.82f, 0.35f, 1.00f);  // RGB(48, 209, 88)
    constexpr ImVec4 Red   = ImVec4(1.00f, 0.26f, 0.27f, 1.00f);  // RGB(255, 66, 69)
    [[maybe_unused]] constexpr ImVec4 Gray4 = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);  // RGB(58, 58, 60)
    [[maybe_unused]] constexpr ImVec4 Gray3 = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);  // RGB(72, 72, 74)
} // namespace Colors

// ═══════════════════════════════════════════════════════════════════════════
// Implementation
// ═══════════════════════════════════════════════════════════════════════════

void RenderMainMenu() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Main Menu", nullptr, windowFlags);

    // Center content
    const ImVec2 windowSize = ImGui::GetWindowSize();
    constexpr float contentHeight = 340.0f;
    constexpr float contentWidth = 360.0f;

    ImGui::SetCursorPosY((windowSize.y - contentHeight) * 0.4f);
    const float offsetX = (windowSize.x - contentWidth) * 0.5f;
    ImGui::SetCursorPosX(offsetX);

    ImGui::BeginGroup();
    {
        // Title
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
        constexpr const char* title = "R-Type Asset Editor";
        const float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX(offsetX + (contentWidth - titleWidth) * 0.5f);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Subtitle (secondary label color)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.92f, 0.96f, 0.55f));
        constexpr const char* subtitle = "Select a tool to get started";
        const float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX(offsetX + (contentWidth - subtitleWidth) * 0.5f);
        ImGui::Text("%s", subtitle);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // Menu buttons
        constexpr float buttonWidth = contentWidth;
        constexpr float buttonHeight = 44.0f;

        ImGui::SetCursorPosX(offsetX);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 12));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

        // JSON Editor Button - Primary action (Blue accent)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Blue);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.65f, 1.00f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.50f, 0.90f, 1.00f));
        if (ImGui::Button("JSON Config Editor", ImVec2(buttonWidth, buttonHeight))) {
            g_state.mode = EditorMode::JsonEditor;
            RefreshFileList();
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Edit game configuration files (enemies, levels, etc.)");
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SetCursorPosX(offsetX);

        // Sprite Editor Button - Secondary action (Green accent)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Green);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.88f, 0.42f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.75f, 0.30f, 1.00f));
        if (ImGui::Button("Sprite Editor", ImVec2(buttonWidth, buttonHeight))) {
            g_state.mode = EditorMode::SpriteEditor;
            RefreshSpriteList();
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Browse and edit sprite assets");
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SetCursorPosX(offsetX);

        // Exit Button - Destructive action (Red accent)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Red);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.35f, 0.36f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.20f, 0.22f, 1.00f));
        if (ImGui::Button("Exit", ImVec2(buttonWidth, buttonHeight))) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }
        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar(2);
    }
    ImGui::EndGroup();

    // Version footer
    ImGui::SetCursorPos(ImVec2(20, windowSize.y - 30));
    ImGui::TextDisabled("v1.0.0");

    ImGui::End();
}

} // namespace AssetEditor
