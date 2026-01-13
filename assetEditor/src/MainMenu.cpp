/**
 * @file MainMenu.cpp
 * @brief Main menu interface implementation
 * 
 * Uses authentic macOS system colors from Apple HIG.
 */

#include "MainMenu.h"
#include "EditorState.h"
#include <imgui.h>
#include <SDL.h>

namespace AssetEditor {

// macOS System Colors (Dark Mode)
namespace {
    const ImVec4 MacBlue   = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    const ImVec4 MacRed    = ImVec4(1.00f, 0.26f, 0.27f, 1.00f);  // RGB(255, 66, 69)
    const ImVec4 MacGray4  = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);  // RGB(58, 58, 60)
    const ImVec4 MacGray3  = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);  // RGB(72, 72, 74)
}

void RenderMainMenu() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("Main Menu", nullptr, windowFlags);
    
    // Center content
    ImVec2 windowSize = ImGui::GetWindowSize();
    float contentHeight = 340.0f;
    float contentWidth = 360.0f;
    
    ImGui::SetCursorPosY((windowSize.y - contentHeight) * 0.4f);
    float offsetX = (windowSize.x - contentWidth) * 0.5f;
    ImGui::SetCursorPosX(offsetX);
    
    ImGui::BeginGroup();
    {
        // Title
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
        const char* title = "R-Type Asset Editor";
        float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX(offsetX + (contentWidth - titleWidth) * 0.5f);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Subtitle (secondary label color)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.92f, 0.96f, 0.55f));
        const char* subtitle = "Select a tool to get started";
        float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX(offsetX + (contentWidth - subtitleWidth) * 0.5f);
        ImGui::Text("%s", subtitle);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Menu buttons
        float buttonWidth = contentWidth;
        float buttonHeight = 44.0f;
        
        ImGui::SetCursorPosX(offsetX);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 12));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        // JSON Editor Button - Primary action (Blue accent)
        ImGui::PushStyleColor(ImGuiCol_Button, MacBlue);
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
        
        // Sprite Editor Button (disabled - gray)
        ImGui::PushStyleColor(ImGuiCol_Button, MacGray4);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MacGray3);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, MacGray3);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.92f, 0.96f, 0.30f));
        ImGui::Button("Sprite Editor", ImVec2(buttonWidth, buttonHeight));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Coming soon...");
        }
        ImGui::PopStyleColor(4);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SetCursorPosX(offsetX);
        
        // Exit Button - macOS Red
        ImGui::PushStyleColor(ImGuiCol_Button, MacRed);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.35f, 0.36f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.90f, 0.20f, 0.22f, 1.00f));
        if (ImGui::Button("Exit", ImVec2(buttonWidth, buttonHeight))) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Version info (quaternary label)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.92f, 0.96f, 0.30f));
        const char* version = "v1.0.0";
        float versionWidth = ImGui::CalcTextSize(version).x;
        ImGui::SetCursorPosX(offsetX + (contentWidth - versionWidth) * 0.5f);
        ImGui::Text("%s", version);
        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();
    
    ImGui::End();
}

} // namespace AssetEditor
