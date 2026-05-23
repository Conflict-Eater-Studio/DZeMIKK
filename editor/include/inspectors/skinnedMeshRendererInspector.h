#ifndef EDITOR_SKINNEDMESH_RENDERER_INSPECTOR_H
#define EDITOR_SKINNEDMESH_RENDERER_INSPECTOR_H

namespace dzemikk {
class SkinnedMeshRenderer;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing SkinnedMeshRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given SkinnedMeshRenderer instance.
 */
class SkinnedMeshRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a SkinnedMeshRenderer component.
     *
     * @param renderer Pointer to the SkinnedMeshRenderer component being inspected.
     * @param ctx Shared inspector context.
     */
    static void draw(dzemikk::SkinnedMeshRenderer* renderer, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif 