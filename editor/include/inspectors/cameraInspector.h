#ifndef EDITOR_CAMERA_INSPECTOR_H
#define EDITOR_CAMERA_INSPECTOR_H

namespace dzemikk {
class Camera;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing Camera components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given Camera instance.
 */
class CameraInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a Camera component.
     *
     * @param camera Pointer to the Camera component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::Camera* camera, const InspectorContext& ctx);

#pragma endregion
  private:
    static void drawPerspective(dzemikk::Camera* camera);
    static void drawOrthographic(dzemikk::Camera* camera);
};

} // namespace editor

#endif