#pragma once

#include "ecs/components/meshRenderer.h"
#include "inspectors/inspectorRegistry.h"

namespace editor {

class MeshRendererInspector {
  public:
    static void draw(dzemikk::MeshRenderer* renderer, const InspectorContext& ctx);

  private:
    enum class ModelSource { Primitive, Custom };
};

} // namespace editor