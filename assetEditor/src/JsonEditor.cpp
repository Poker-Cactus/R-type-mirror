/**
 * @file JsonEditor.cpp
 * @brief JSON editing functionality implementation
 * 
 * Uses authentic macOS system colors from Apple HIG.
 */

#include "JsonEditor.h"
#include "EditorState.h"
#include <imgui.h>
#include <vector>
#include <cstring>

namespace AssetEditor {

// macOS System Colors (Dark Mode)
namespace {
    const ImVec4 MacOrange = ImVec4(1.00f, 0.57f, 0.19f, 1.00f);  // RGB(255, 146, 48)
    const ImVec4 MacYellow = ImVec4(1.00f, 0.84f, 0.04f, 1.00f);  // RGB(255, 214, 10)
    const ImVec4 MacBlue   = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
}

std::string GetItemLabel(const json& item, size_t index) {
    static const std::vector<std::string> labelFields = {"name", "id", "title", "label", "key"};
    
    for (const auto& field : labelFields) {
        if (item.contains(field) && item[field].is_string()) {
            return item[field].get<std::string>();
        }
    }
    return "Item " + std::to_string(index);
}

void EditJsonValue(const std::string& key, json& value) {
    ImGui::PushID(key.c_str());
    
    if (value.is_boolean()) {
        bool b = value.get<bool>();
        if (ImGui::Checkbox(key.c_str(), &b)) {
            value = b;
            g_state.modified = true;
        }
    }
    else if (value.is_number_integer()) {
        int i = value.get<int>();
        if (ImGui::InputInt(key.c_str(), &i)) {
            value = i;
            g_state.modified = true;
        }
    }
    else if (value.is_number_float()) {
        float f = value.get<float>();
        if (ImGui::DragFloat(key.c_str(), &f, 0.1f)) {
            value = f;
            g_state.modified = true;
        }
    }
    else if (value.is_string()) {
        std::string s = value.get<std::string>();
        char buffer[256];
        strncpy(buffer, s.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputText(key.c_str(), buffer, sizeof(buffer))) {
            value = std::string(buffer);
            g_state.modified = true;
        }
    }
    
    ImGui::PopID();
}

void RenderJsonObject(const std::string& label, json& obj) {
    if (!obj.is_object()) return;
    
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

void RenderListItem(json& item, size_t index) {
    std::string label = GetItemLabel(item, index);
    
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
    json* mainArray = nullptr;
    
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
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("JSON Editor", nullptr, windowFlags);
    
    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        // Back button with subtle blue accent
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.35f, 0.55f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MacBlue);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.45f, 0.85f, 1.00f));
        if (ImGui::Button("<< Menu")) {
            g_state.mode = EditorMode::MainMenu;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        // Save Button with macOS Orange accent
        if (!g_state.selectedFile.empty()) {
            bool wasModified = g_state.modified;
            
            if (wasModified) {
                ImGui::PushStyleColor(ImGuiCol_Button, MacOrange);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.65f, 0.30f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.90f, 0.50f, 0.15f, 1.00f));
            }
            
            if (ImGui::Button("Save")) {
                SaveFile();
            }
            
            if (wasModified) {
                ImGui::PopStyleColor(3);
            }
            
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            
            if (g_state.modified) {
                ImGui::TextColored(MacYellow, "%s *", g_state.selectedFile.c_str());
            } else {
                ImGui::Text("%s", g_state.selectedFile.c_str());
            }
        }
        
        ImGui::EndMenuBar();
    }
    
    // Main Layout: Left Panel (Files) + Right Panel (Editor)
    float panelWidth = ImGui::GetContentRegionAvail().x * 0.20f;
    
    // Left Panel: File List
    ImGui::BeginChild("FileList", ImVec2(panelWidth, 0), true);
    {
        ImGui::Text("Config Files");
        ImGui::Separator();
        
        if (ImGui::Button("Refresh", ImVec2(-1, 0))) {
            RefreshFileList();
        }
        
        ImGui::Spacing();
        
        for (const auto& file : g_state.jsonFiles) {
            bool isSelected = (file == g_state.selectedFile);
            if (ImGui::Selectable(file.c_str(), isSelected)) {
                if (file != g_state.selectedFile) {
                    LoadFile(file);
                }
            }
        }
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Right Panel: JSON Editor
    ImGui::BeginChild("Editor", ImVec2(0, 0), true);
    {
        RenderJsonEditor();
    }
    ImGui::EndChild();
    
    ImGui::End();
}

} // namespace AssetEditor
