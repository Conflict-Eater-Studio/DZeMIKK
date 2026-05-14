#pragma once
#include "ecs/components/ui/canvas.h"
#include "inspectorRegistry.h"

namespace editor {
class CanvasInspector {
  public:
    static void draw(dzemikk::Canvas* canvas, const InspectorContext& ctx);
};

} // namespace editor

