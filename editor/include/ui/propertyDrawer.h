#ifndef EDITOR_PROPERTY_DRAWER_INSPECTOR_H
#define EDITOR_PROPERTY_DRAWER_INSPECTOR_H

#include "inspectors/inspectorRegistry.h"

#include <assetManager/assetHandle.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/components/ui/iUIInteractable.h>
#include <renderer/font.h>
#include <renderer/model.h>
#include <renderer/shader.h>
#include <renderer/texture.h>

#include <glm/glm.hpp>
#include <imgui.h>
#include <string>

namespace editor {

/**
 * @brief Utility class for drawing reusable ImGui property widgets.
 *
 * Provides standardized UI controls for editing engine properties
 * such as primitives, vectors, assets, and engine-specific components.
 */
class PropertyDrawer {
  public:
#pragma region Primitives

    /**
     * @brief Draws a float input field.
     *
     * @param label UI label.
     * @param value Reference to the float value.
     * @param speed Drag speed for ImGui control.
     * @return True if value was modified.
     */
    static bool drawFloat(const std::string& label, float& value, float speed = 0.1F);

    /**
     * @brief Draws a float input field with clamping range.
     *
     * @param label UI label.
     * @param value Reference to the float value.
     * @param min Minimum allowed value.
     * @param max Maximum allowed value.
     * @param speed Drag speed for ImGui control.
     * @return True if value was modified.
     */
    static bool drawFloat(const std::string& label, float& value, float min, float max,
                          float speed = 0.1F);

    /**
     * @brief Draws an integer input field.
     *
     * @param label UI label.
     * @param value Reference to the integer value.
     * @param speed Drag speed for ImGui control.
     * @return True if value was modified.
     */
    static bool drawInt(const std::string& label, int& value, int speed = 1);

    /**
     * @brief Draws an integer input field with clamping range.
     *
     * @param label UI label.
     * @param value Reference to the integer value.
     * @param min Minimum allowed value.
     * @param max Maximum allowed value.
     * @return True if value was modified.
     */
    static bool drawInt(const std::string& label, int& value, int min, int max);

    /**
     * @brief Draws a boolean checkbox.
     *
     * @param label UI label.
     * @param value Reference to the boolean value.
     * @return True if value was modified.
     */
    static bool drawBool(const std::string& label, bool& value);

#pragma endregion

#pragma region Math Types

    /**
     * @brief Draws a 2D vector input field.
     *
     * @param label UI label.
     * @param value Reference to the vec2 value.
     * @param resetValue Default reset value for components.
     * @return True if value was modified.
     */
    static bool drawVec2(const std::string& label, glm::vec2& value, float resetValue = 0.0F);

    /**
     * @brief Draws a 3D vector input field.
     *
     * @param label UI label.
     * @param value Reference to the vec3 value.
     * @param resetValue Default reset value for components.
     * @return True if value was modified.
     */
    static bool drawVec3(const std::string& label, glm::vec3& value, float resetValue = 0.0F);

    /**
     * @brief Draws a color picker (RGBA).
     *
     * @param label UI label.
     * @param color Reference to color vector.
     * @return True if value was modified.
     */
    static bool drawColor(const std::string& label, glm::vec4& color);

#pragma endregion

#pragma region Assets

    /**
     * @brief Draws a model asset selector.
     *
     * @param label UI label.
     * @param handle Model asset handle reference.
     * @param ctx Inspector context for asset access.
     * @return True if asset was changed.
     */
    static bool drawModel(const std::string& label, dzemikk::AssetHandle<dzemikk::Model>& handle,
                          const InspectorContext& ctx);

    /**
     * @brief Draws a shader asset selector.
     *
     * @param label UI label.
     * @param handle Shader asset handle reference.
     * @param ctx Inspector context for asset access.
     * @return True if asset was changed.
     */
    static bool drawShader(const std::string& label, dzemikk::AssetHandle<dzemikk::Shader>& handle,
                           const InspectorContext& ctx);

    /**
     * @brief Draws a texture asset selector.
     *
     * @param label UI label.
     * @param handle Texture asset handle reference.
     * @param ctx Inspector context for asset access.
     * @return True if asset was changed.
     */
    static bool drawTexture(const std::string& label,
                            dzemikk::AssetHandle<dzemikk::Texture>& handle,
                            const InspectorContext& ctx);

    /**
     * @brief Draws a font asset selector.
     *
     * @param label UI label.
     * @param handle Font asset handle reference.
     * @param ctx Inspector context for asset access.
     * @return True if asset was changed.
     */
    static bool drawFont(const std::string& label, dzemikk::AssetHandle<dzemikk::Font>& handle,
                         const InspectorContext& ctx);

#pragma endregion

#pragma region ECS / Engine Specific

    /**
     * @brief Draws material editor for a MeshRenderer component.
     *
     * @param label UI label.
     * @param renderer MeshRenderer pointer.
     * @param ctx Inspector context.
     * @return True if materials were modified.
     */
    static bool drawMaterials(const std::string& label, dzemikk::MeshRenderer* renderer,
                              const InspectorContext& ctx);

    /**
     * @brief Draws UI event bindings for interactable UI components.
     *
     * @param label UI label.
     * @param interactable UI interactable component pointer.
     * @param ctx Inspector context.
     * @return True if events were modified.
     */
    static bool drawUIEvents(const std::string& label, dzemikk::IUIInteractable* interactable,
                             const InspectorContext& ctx);

#pragma endregion

#pragma region Templates

    /**
     * @brief Draws an enum selector using ImGui combo box.
     *
     * @tparam Enum Enum type.
     * @param label UI label.
     * @param value Reference to enum value.
     * @param items Array of enum string names.
     * @param itemCount Number of enum items.
     * @return True if value was changed.
     */
    template <typename Enum>
    static bool drawEnum(const std::string& label, Enum& value, const char* const items[],
                         int itemCount);

#pragma endregion
};

} // namespace editor

#endif

// ================= Template Implementation =================
template <typename Enum>
bool editor::PropertyDrawer::drawEnum(const std::string& label, Enum& value,
                                      const char* const items[], int itemCount) {

    int current = static_cast<int>(value);

    if (ImGui::Combo(label.c_str(), &current, items, itemCount)) {
        value = static_cast<Enum>(current);
        return true;
    }

    return false;
}