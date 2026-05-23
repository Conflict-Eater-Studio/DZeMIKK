#ifndef EDITOR_POINT_LIGHT_INSPECTOR_H
#define EDITOR_POINT_LIGHT_INSPECTOR_H

namespace dzemikk {
class PointLight;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing PointLight components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given PointLight instance.
 */
class PointLightInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a PointLight component.
     *
     * @param light Pointer to the PointLight component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::PointLight* light, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif