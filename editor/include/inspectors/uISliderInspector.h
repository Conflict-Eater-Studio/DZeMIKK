#ifndef EDITOR_UISLIDER_INSPECTOR_H
#define EDITOR_UISLIDER_INSPECTOR_H

namespace dzemikk {
class UISlider;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing UISlider components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given UISlider instance.
 */
class UISliderInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a UISlider component.
     *
     * @param slider Pointer to the UISlider component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::UISlider* slider, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif