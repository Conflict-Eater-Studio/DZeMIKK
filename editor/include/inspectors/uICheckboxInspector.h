#ifndef EDITOR_UICHECKBOX_INSPECTOR_H
#define EDITOR_UICHECKBOX_INSPECTOR_H

namespace dzemikk {
class UICheckbox;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing UICheckbox components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given UICheckbox instance.
 */
class UICheckboxInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a UICheckbox component.
     *
     * @param checkbox Pointer to the UICheckbox component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::UICheckbox* checkbox, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif