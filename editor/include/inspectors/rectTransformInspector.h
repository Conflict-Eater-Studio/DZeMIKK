#ifndef EDITOR_RECTTRANSFORM_INSPECTOR_H
#define EDITOR_RECTTRANSFORM_INSPECTOR_H

namespace dzemikk {
class RectTransform;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing RectTransform components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given RectTransform instance.
 */
class RectTransformInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a RectTransform component.
     *
     * @param transform Pointer to the RectTransform component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::RectTransform* transform, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif