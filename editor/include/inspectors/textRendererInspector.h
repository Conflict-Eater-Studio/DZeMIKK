#ifndef EDITOR_TEXTRENDERER_INSPECTOR_H
#define EDITOR_TEXTRENDERER_INSPECTOR_H

namespace dzemikk {
class TextRenderer;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing TextRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given TextRenderer instance.
 */
class TextRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a TextRenderer component.
     *
     * @param textRenderer Pointer to the TextRenderer component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::TextRenderer* textRenderer, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif // EDITOR_TEXTRENDERERINSPECTOR_H