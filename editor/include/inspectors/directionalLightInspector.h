#ifndef EDITOR_DIRECTIONAL_LIGHT_INSPECTOR_H
#define EDITOR_DIRECTIONAL_LIGHT_INSPECTOR_H

namespace dzemikk {
class DirectionalLight;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing DirectionalLight components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given DirectionalLight instance.
 */
class DirectionalLightInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a DirectionalLight component.
     *
     * @param light Pointer to the DirectionalLight component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::DirectionalLight* light, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif