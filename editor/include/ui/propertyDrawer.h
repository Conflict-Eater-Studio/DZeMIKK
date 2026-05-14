#pragma once

#include <glm/glm.hpp>
#include <string>

#include "assetManager/assetHandle.h"
#include "renderer/model.h"
#include "renderer/shader.h"

#include "inspectors/inspectorRegistry.h"

#include "ecs/components/meshRenderer.h"

namespace editor {
class PropertyDrawer {
  public:
    static bool drawFloat(const std::string& label, float& value, float speed = 0.1F);
    static bool drawFloat(const std::string& label, float& value, float min, float max,
                          float speed = 0.1f);

    static bool drawVec3(const std::string& label, glm::vec3& value, float resetValue = 0.0F);

    static bool drawBool(const std::string& label, bool& value);

    static bool drawModel(const std::string& label, dzemikk::AssetHandle<dzemikk::Model>& handle,
                          const InspectorContext& ctx);

    static bool drawShader(const std::string& label, dzemikk::AssetHandle<dzemikk::Shader>& handle,
                           const InspectorContext& ctx);

    static bool drawColor(const std::string& label, glm::vec4& color);

    static bool drawMaterials(const std::string& label, dzemikk::MeshRenderer* renderer,
                              const InspectorContext& ctx);
};

} // namespace editor