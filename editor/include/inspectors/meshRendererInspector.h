#ifndef EDITOR_MESH_RENDERER_INSPECTOR_H
#define EDITOR_MESH_RENDERER_INSPECTOR_H

#include <cstdint>

namespace dzemikk {
class MeshRenderer;
}

namespace editor {

/**
 * @brief Provides UI utilities for inspecting and editing MeshRenderer components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given MeshRenderer instance.
 */
struct InspectorContext;

class MeshRendererInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a MeshRenderer component.
     *
     * @param renderer Pointer to the MeshRenderer component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::MeshRenderer* renderer, const InspectorContext& ctx);

#pragma endregion

  private:
#pragma region Types

    /**
     * @brief Specifies the source type of the mesh model.
     *
     * Stored as a compact 8-bit enum to reduce memory usage.
     */
    enum class ModelSource : std::uint8_t { Primitive, Custom };

#pragma endregion
};

} // namespace editor

#endif