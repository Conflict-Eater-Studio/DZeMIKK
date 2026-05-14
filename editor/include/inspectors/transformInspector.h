#pragma once
#include "inspectorRegistry.h"

namespace dzemikk {
class Transform;
}

namespace editor {

class TransformInspector {
  public:
    static void draw(dzemikk::Transform* transform, const InspectorContext& ctx);
};

} // namespace editor