#pragma once
#include "ecs/components/light/directionalLight.h"
#include "inspectorRegistry.h"

namespace editor {
class DirectionalLightInspector {
  public:
    static void draw(dzemikk::DirectionalLight* light, const InspectorContext& ctx);
};

} // namespace editor

