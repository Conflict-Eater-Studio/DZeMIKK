#pragma once

namespace dzemikk {
class Transform;
}

namespace editor {

class TransformInspector {
  public:
    static void draw(dzemikk::Transform* transform);
};

} // namespace editor