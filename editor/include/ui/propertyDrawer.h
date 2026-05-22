#pragma once

#include <glm/glm.hpp>
#include <string>

#include "assetManager/assetHandle.h"
#include "renderer/model.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/font.h"

#include "inspectors/inspectorRegistry.h"

#include "ecs/components/meshRenderer.h"

#include <imgui.h>
#include <ecs/components/ui/iUIInteractable.h>

namespace editor {
class PropertyDrawer {
  public:
    static bool drawFloat(const std::string& label, float& value, float speed = 0.1F);
    static bool drawFloat(const std::string& label, float& value, float min, float max,
                          float speed = 0.1f);

    static bool drawInt(const std::string& label, int& value, int speed = 1);
    static bool drawInt(const std::string& label, int& value, int min, int max);

    static bool drawVec2(const std::string& label, glm::vec2& value, float resetValue = 0.0F);
    static bool drawVec3(const std::string& label, glm::vec3& value, float resetValue = 0.0F);

    static bool drawBool(const std::string& label, bool& value);

    static bool drawModel(const std::string& label, dzemikk::AssetHandle<dzemikk::Model>& handle,
                          const InspectorContext& ctx);

    static bool drawShader(const std::string& label, dzemikk::AssetHandle<dzemikk::Shader>& handle,
                           const InspectorContext& ctx);

    static bool drawColor(const std::string& label, glm::vec4& color);

    static bool drawMaterials(const std::string& label, dzemikk::MeshRenderer* renderer,
                              const InspectorContext& ctx);

    template <typename Enum>
    static bool drawEnum(const std::string& label, Enum& value, const char* const items[],
                         int itemCount);

    static bool drawTexture(const std::string& label,
                            dzemikk::AssetHandle<dzemikk::Texture>& handle,
                            const InspectorContext& ctx);

    static bool drawFont(const std::string& label, dzemikk::AssetHandle<dzemikk::Font>& handle,
                         const InspectorContext& ctx);

    static bool drawUIEvents(const std::string& label, dzemikk::IUIInteractable* interactable,
                             const InspectorContext& ctx);
};

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
} // namespace editor