#pragma once
#include "ecs/components/light/spotLight.h"
#include "inspectorRegistry.h"

namespace editor {
class SpotLightInspector {
  public:
    static void draw(dzemikk::SpotLight* light, const InspectorContext& ctx);
};

} // namespace editor

