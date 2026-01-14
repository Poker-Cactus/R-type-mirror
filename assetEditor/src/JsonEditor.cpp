/**
 * @file JsonEditor.cpp
 * @brief JSON editing functionality implementation
 *
 * Uses authentic macOS system colors from Apple HIG.
 */

#include "JsonEditor.hpp"
#include "EditorState.hpp"

#include <imgui.h>

#include <cstring>
#include <vector>

namespace AssetEditor {

// ═══════════════════════════════════════════════════════════════════════════
// macOS System Colors (Dark Mode)
// ═══════════════════════════════════════════════════════════════════════════

namespace Colors {
    constexpr ImVec4 Orange = ImVec4(1.00f, 0.57f, 0.19f, 1.00f);  // RGB(255, 146, 48)
    [[maybe_unused]] constexpr ImVec4 Yellow = ImVec4(1.00f, 0.84f, 0.04f, 1.00f);  // RGB(255, 214, 10)
    [[maybe_unused]] constexpr ImVec4 Blue   = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    [[maybe_unused]] constexpr ImVec4 Red    = ImVec4(1.00f, 0.26f, 0.27f, 1.00f);  // RGB(255, 66, 69)
} // namespace Colors

// ═══════════════════════════════════════════════════════════════════════════
// Label Fields for Smart Detection
// ═══════════════════════════════════════════════════════════════════════════

namespace {
    const std::vector<std::string> kLabelFields = {"name", "id", "title", "label", "key"};
} // namespace

// ═══════════════════════════════════════════════════════════════════════════
// Implementation
// ═══════════════════════════════════════════════════════════════════════════

std::string GetItemLabel(const Json& item, size_t index) {
    for (const auto& field : kLabelFields) {
        if (item.contains(field) && item[field].is_string()) {
            return item[field].get<std::string>();
        }
    }
    return "Item " + std::to_string(index);
}

void EditJsonValue(const std::string& key, Json& value) {
    ImGui::PushID(key.c_str());

    if (value.is_boolean()) {
        bool b = value.get<bool>();
        if (ImGui::Checkbox(key.c_str(), &b)) {
            value = b;
            g_state.modified = true;
        }
    } else if (value.is_number_integer()) {
        int i = value.get<int>();
        if (ImGui::InputInt(key.c_str(), &i)) {
            value = i;
            g_state.modified = true;
        }
    } else if (value.is_number_float()) {
        float f = value.get<float>();
        if (ImGui::DragFloat(key.c_str(), &f, 0.1f)) {
            value = f;
            g_state.modified = true;
        }
    } else if (value.is_string()) {
        std::string s = value.get<std::string>();
        char buffer[256];
        std::strncpy(buffer, s.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputText(key.c_str(), buffer, sizeof(buffer))) {
            value = std::string(buffer);
            g_state.modified = true;
        }
    }

    ImGui::PopID();
}

void RenderJsonObject(const std::string& label, Json& obj) {
    if (!obj.is_object()) {
        return;
    }

    if (ImGui::TreeNode(label.c_str())) {
        for (auto& [key, value] : obj.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }
        ImGui::TreePop();
    }
}

void RenderListItem(Json& item, size_t index) {
    const std::string label = GetItemLabel(item, index);

    ImGui::PushID(static_cast<int>(index));

    if (ImGui::CollapsingHeader(label.c_str())) {
        ImGui::Indent(10.0f);

        for (auto& [key, value] : item.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }

        ImGui::Unindent(10.0f);
    }

    ImGui::PopID();
}

void RenderJsonEditor() {
    if (g_state.currentJson.is_null()) {
        ImGui::TextDisabled("No file loaded. Select a JSON file from the left panel.");
        return;
    }

    // Smart detection: if root is object with a single array, dive into it
    std::string mainArrayKey;
    Json* mainArray = nullptr;

    if (g_state.currentJson.is_object()) {
        for (auto& [key, value] : g_state.currentJson.items()) {
            if (value.is_array()) {
                mainArrayKey = key;
                mainArray = &value;
                break;
            }
        }
    }

    if (mainArray && !mainArray->empty()) {
        // Direct list display mode
        ImGui::Text("Editing: %s (%zu items)", mainArrayKey.c_str(), mainArray->size());
        ImGui::Separator();

        for (size_t i = 0; i < mainArray->size(); ++i) {
            RenderListItem((*mainArray)[i], i);
        }
    } else if (g_state.currentJson.is_object()) {
        // Fallback: render as generic object
        ImGui::Text("Object Editor");
        ImGui::Separator();

        for (auto& [key, value] : g_state.currentJson.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }
    } else if (g_state.currentJson.is_array()) {
        // Direct array at root
        ImGui::Text("Array Editor (%zu items)", g_state.currentJson.size());
        ImGui::Separator();

        for (size_t i = 0; i < g_state.currentJson.size(); ++i) {
            RenderListItem(g_state.currentJson[i], i);
        }
    }
}

void RenderJsonEditorUI() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("JSON Editor", nullptr, windowFlags);

    // Top bar with back button
    if (ImGui::Button("← Back to Menu")) {
        g_state.mode = EditorMode::MainMenu;
        g_state.currentJson = Json();
        g_state.selectedFile.clear();
    }

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    if (!g_state.selectedFile.empty()) {
        ImGui::Text("%s", g_state.selectedFile.c_str());
        if (g_state.modified) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::Orange);
            ImGui::Text("(modified)");
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            (void)SaveFile();
        }
    } else {
        ImGui::TextDisabled("No file selected");
    }

    ImGui::Separator();

    // Main content: File list (20%) + Editor (80%)
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    constexpr float listWidthRatio = 0.20f;

    // Left panel: File list
    ImGui::BeginChild("FileList", ImVec2(windowWidth * listWidthRatio, 0), true);
    {
        ImGui::Text("Config Files");
        ImGui::Separator();

        for (const auto& file : g_state.jsonFiles) {
            const bool isSelected = (file == g_state.selectedFile);
            if (ImGui::Selectable(file.c_str(), isSelected)) {
                (void)LoadFile(file);
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel: Editor
    ImGui::BeginChild("Editor", ImVec2(0, 0), true);
    {
        RenderJsonEditor();
    }
    ImGui::EndChild();

    ImGui::End();
}

} // namespace AssetEditor
