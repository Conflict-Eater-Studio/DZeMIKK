#pragma once
#include "ecs/components/ui/uiButton.h"
#include "inspectorRegistry.h"

namespace editor {
class UIButtonInspector {
  public:
    static void draw(dzemikk::UIButton* uiButton, const InspectorContext& ctx);
};

} // namespace editor
