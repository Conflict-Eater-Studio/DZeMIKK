#ifndef EDITOR_SKINNEDMESHRENDERERINSPECTOR_H
#define EDITOR_SKINNEDMESHRENDERERINSPECTOR_H

#include "ecs/components/skinnedMeshRenderer.h"

#include "inspectorRegistry.h"

namespace editor {

class SkinnedMeshRendererInspector {
  public:
    static void draw(dzemikk::SkinnedMeshRenderer* renderer, const InspectorContext& ctx);
};

} // namespace editor

#endif // EDITOR_SKINNEDMESHRENDERERINSPECTOR_H