#ifndef EDITOR_SPRITE_RENDERER_INSPECTOR_H
#define EDITOR_SPRITE_RENDERER_INSPECTOR_H

#include "inspectors/inspectorRegistry.h"

namespace dzemikk {
class SpriteRenderer;
}

namespace editor {

class SpriteRendererInspector {
  public:
    static void draw(dzemikk::SpriteRenderer* spriteRenderer, const InspectorContext& ctx);
};

} // namespace editor

#endif // EDITOR_SPRITE_RENDERER_INSPECTOR_H