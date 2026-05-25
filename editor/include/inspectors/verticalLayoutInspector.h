#ifndef EDITOR_VERTICAL_LAYOUT_INSPECTOR_H
#define EDITOR_VERTICAL_LAYOUT_INSPECTOR_H

namespace dzemikk {
class VerticalLayout;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing VerticalLayout components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given VerticalLayout instance.
 */
class VerticalLayoutInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a VerticalLayout component.
     *
     * @param layout Pointer to the VerticalLayout component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::VerticalLayout* layout, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif