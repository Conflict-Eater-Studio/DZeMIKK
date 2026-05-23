#ifndef EDITOR_COLLIDER_INSPECTOR_H
#define EDITOR_COLLIDER_INSPECTOR_H

namespace dzemikk {
class Collider;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing Collider components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given Collider instance.
 */
class ColliderInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for a Collider component.
     *
     * @param collider Pointer to the Collider component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::Collider* collider, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif // EDITOR_COLLIDERINSPECTOR_H