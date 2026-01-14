/**
 * @file JsonEditor.hpp
 * @brief JSON editing functionality
 *
 * Provides recursive JSON editing with automatic type detection.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace AssetEditor {

using Json = nlohmann::json;

/**
 * @brief Get display label for a JSON array item
 * @param item JSON object to extract label from
 * @param index Fallback index if no label found
 * @return Display string (e.g., "Enemy: Drone" or "Item #3")
 */
[[nodiscard]] std::string GetItemLabel(const Json& item, size_t index);

/**
 * @brief Render editing widget for a single JSON value
 * @param key Property name
 * @param value JSON value to edit (modified in-place)
 */
void EditJsonValue(const std::string& key, Json& value);

/**
 * @brief Render a JSON object as editable fields
 * @param label Display label for the object
 * @param obj JSON object to edit
 */
void RenderJsonObject(const std::string& label, Json& obj);

/**
 * @brief Render a single item in a JSON array
 * @param item JSON object representing the item
 * @param index Item position in array
 */
void RenderListItem(Json& item, size_t index);

/**
 * @brief Render the main JSON content editor
 *
 * Automatically detects main arrays (e.g., "enemies": [...])
 * and displays them directly as collapsible items.
 */
void RenderJsonEditor();

/**
 * @brief Render the complete JSON editor UI
 *
 * Shows file list (20%), editor panel (80%), and Back/Save buttons.
 */
void RenderJsonEditorUI();

} // namespace AssetEditor
