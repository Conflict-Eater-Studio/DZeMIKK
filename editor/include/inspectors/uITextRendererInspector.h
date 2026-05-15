#pragma once
#include "inspectorRegistry.h"

#ifndef EDITOR_UITEXTRENDERERINSPECTOR_H
#define EDITOR_UITEXTRENDERERINSPECTOR_H

namespace dzemikk {
class UITextRenderer;
}

namespace editor {

class UITextRendererInspector{
  public:
    static void draw(dzemikk::UITextRenderer* textRenderer, const InspectorContext& ctx);
};

} // namespace editor

#endif // EDITOR_UITEXTRENDERERINSPECTOR_H