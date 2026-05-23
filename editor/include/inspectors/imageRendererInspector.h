#ifndef EDITOR_IMAGE_RENDERER_INSPECTOR_H
#define EDITOR_IMAGE_RENDERER_INSPECTOR_H

namespace dzemikk {
class ImageRenderer;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing ImageRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given ImageRenderer instance.
 */
class ImageRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for an ImageRenderer component.
     *
     * @param renderer Pointer to the ImageRenderer component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::ImageRenderer* renderer, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif