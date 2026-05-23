#ifndef EDITOR_CANVAS_INSPECTOR_H
#define EDITOR_CANVAS_INSPECTOR_H

namespace dzemikk {
class Canvas;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing Canvas components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given Canvas instance.
 */
class CanvasInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a Canvas component.
     *
     * @param canvas Pointer to the Canvas component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::Canvas* canvas, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif