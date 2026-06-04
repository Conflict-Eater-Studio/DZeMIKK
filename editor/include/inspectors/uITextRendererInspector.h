#ifndef EDITOR_UITEXT_RENDERER_INSPECTOR_H
#define EDITOR_UITEXT_RENDERER_INSPECTOR_H

namespace dzemikk {
class UITextRenderer;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing UITextRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given UITextRenderer instance.
 */
class UITextRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a UITextRenderer component.
     *
     * @param textRenderer Pointer to the UITextRenderer component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::UITextRenderer* textRenderer, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif 