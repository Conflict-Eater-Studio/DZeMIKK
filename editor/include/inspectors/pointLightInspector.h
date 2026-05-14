#pragma once
#include "ecs/components/light/pointLight.h"
#include "inspectorRegistry.h"

namespace editor {
class PointLightInspector {
  public:
    static void draw(dzemikk::PointLight* light, const InspectorContext& ctx);
};

} // namespace editor
