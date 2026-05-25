#ifndef EDITOR_UIDROPDOWN_INSPECTOR_H
#define EDITOR_UIDROPDOWN_INSPECTOR_H

namespace dzemikk {
class UIDropdown;
}

namespace editor {

struct InspectorContext;
/**
 * @brief Provides UI utilities for inspecting and editing UIDropdown components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given UIDropdown instance.
 */
class UIDropdownInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a UIDropdown component.
     *
     * @param dropdown Pointer to the UIDropdown component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::UIDropdown* dropdown, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif