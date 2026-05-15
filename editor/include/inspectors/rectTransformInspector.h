#pragma once
#include "ecs/components/ui/rectTransform.h"
#include "inspectorRegistry.h"

namespace editor {
class RectTransformInspector {
  public:
    static void draw(dzemikk::RectTransform* transform, const InspectorContext& ctx);
};

} // namespace editor
