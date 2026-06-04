#ifndef EDITOR_SPRITE_RENDERER_INSPECTOR_H
#define EDITOR_SPRITE_RENDERER_INSPECTOR_H

namespace dzemikk {
class SpriteRenderer;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing SpriteRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given SpriteRenderer instance.
 */
class SpriteRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a SpriteRenderer component.
     *
     * @param spriteRenderer Pointer to the SpriteRenderer component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::SpriteRenderer* spriteRenderer, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif // EDITOR_SPRITE_RENDERER_INSPECTOR_H