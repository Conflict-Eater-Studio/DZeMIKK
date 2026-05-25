#ifndef EDITOR_HORIZONTAL_LAYOUT_INSPECTOR_H
#define EDITOR_HORIZONTAL_LAYOUT_INSPECTOR_H

namespace dzemikk {
class HorizontalLayout;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing HorizontalLayout components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given HorizontalLayout instance.
 */
class HorizontalLayoutInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a HorizontalLayout component.
     *
     * @param layout Pointer to the HorizontalLayout component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::HorizontalLayout* layout, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif