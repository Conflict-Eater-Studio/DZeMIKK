#ifndef EDITOR_CAMERAINSPECTOR_H
#define EDITOR_CAMERAINSPECTOR_H

namespace dzemikk {
class Camera;
}

namespace editor {

struct InspectorContext;

class CameraInspector {
  public:
    static void draw(dzemikk::Camera* camera, const InspectorContext& ctx);
};

} // namespace editor

#endif