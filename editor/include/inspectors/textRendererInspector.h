#ifndef EDITOR_TEXTRENDERERINSPECTOR_H
#define EDITOR_TEXTRENDERERINSPECTOR_H

#include "ecs/components/textRenderer.h"
#include "inspectorRegistry.h"

namespace editor {

class TextRendererInspector {
  public:
    static void draw(dzemikk::TextRenderer* textRenderer, const InspectorContext& ctx);
};

} // namespace editor

#endif // EDITOR_TEXTRENDERERINSPECTOR_H