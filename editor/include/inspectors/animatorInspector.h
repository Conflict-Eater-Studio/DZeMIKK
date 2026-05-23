#ifndef EDITOR_ANIMATOR_INSPECTOR_H
#define EDITOR_ANIMATOR_INSPECTOR_H

namespace dzemikk {
class Animator;
}

namespace editor {

struct InspectorContext;

/**
 * @brief Provides UI utilities for inspecting and editing Animator components.
 *
 * Responsible for rendering the editor inspector interface
 * for a given Animator instance.
 */
class AnimatorInspector {
  public:
#pragma region Draw

    /**
     * @brief Draws the inspector UI for an Animator component.
     *
     * @param animator Pointer to the Animator component being inspected.
     * @param ctx Shared inspector drawing context.
     */
    static void draw(dzemikk::Animator* animator, const InspectorContext& ctx);

#pragma endregion
};

} // namespace editor

#endif