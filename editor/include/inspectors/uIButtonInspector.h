#ifndef EDITOR_UIBUTTON_INSPECTOR_H
#define EDITOR_UIBUTTON_INSPECTOR_H

namespace dzemikk {
class UIButton;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing UIButton components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given UIButton instance.
 */
class UIButtonInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a UIButton component.
     *
     * @param uiButton Pointer to the UIButton component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::UIButton* uiButton, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif