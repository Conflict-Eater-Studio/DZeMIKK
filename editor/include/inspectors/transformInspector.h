#ifndef EDITOR_TRANSFORM_INSPECTOR_H
#define EDITOR_TRANSFORM_INSPECTOR_H

namespace dzemikk {
class Transform;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing Transform components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given Transform instance.
 */
class TransformInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a Transform component.
     *
     * @param transform Pointer to the Transform component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::Transform* transform, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif