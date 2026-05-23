#ifndef EDITOR_GRID_LAYOUT_INSPECTOR_H
#define EDITOR_GRID_LAYOUT_INSPECTOR_H

namespace dzemikk {
class GridLayout;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing GridLayout components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given GridLayout instance.
 */
class GridLayoutInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a GridLayout component.
     *
     * @param layout Pointer to the GridLayout component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::GridLayout* layout, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif