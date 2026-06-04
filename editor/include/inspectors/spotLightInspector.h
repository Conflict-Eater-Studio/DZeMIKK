#ifndef EDITOR_SPOT_LIGHT_INSPECTOR_H
#define EDITOR_SPOT_LIGHT_INSPECTOR_H

namespace dzemikk {
class SpotLight;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing SpotLight components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given SpotLight instance.
 */
class SpotLightInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a SpotLight component.
     *
     * @param light Pointer to the SpotLight component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::SpotLight* light, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif