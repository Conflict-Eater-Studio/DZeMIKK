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
     * @brief Draws the full Animator inspector UI.
     *
     * This is the main entry point for rendering the Animator component inspector.
     * It orchestrates all sub-sections such as state machine, playback state,
     * parameters, and debug tools.
     *
     * @param animator Pointer to the Animator component being inspected.
     * @param ctx Shared inspector drawing context (selection, editor state, etc.).
     */
    static void draw(dzemikk::Animator* animator, const InspectorContext& ctx);

    /**
     * @brief Draws the State Machine assignment section.
     *
     * Allows viewing the currently assigned AnimationStateMachine and
     * creating or replacing it if none exists.
     *
     * @param animator Pointer to the Animator component.
     */
    static void drawAnimationStateMachineSection(dzemikk::Animator* animator);

    /**
     * @brief Draws the current playback state information.
     *
     * Displays the active animation state name and current playback time.
     *
     * @param animator Pointer to the Animator component.
     */
    static void drawCurrentState(dzemikk::Animator* animator);

    /**
     * @brief Draws all animation parameters grouped by type.
     *
     * Renders editable float, bool, and int parameters that drive
     * animation state transitions.
     *
     * @param animator Pointer to the Animator component.
     */
    static void drawParameters(dzemikk::Animator* animator);

    /**
     * @brief UI for adding new animation parameters.
     *
     * Provides input fields for parameter name, type selection,
     * and default value creation.
     *
     * @param animator Pointer to the Animator component.
     */
    static void drawAddParameter(dzemikk::Animator* animator);

    /**
     * @brief Draws debug and testing controls for the Animator.
     *
     * Includes manual state playback controls and developer-only utilities
     * for testing animation behavior at runtime.
     *
     * @param animator Pointer to the Animator component.
     */
    static void drawDebug(dzemikk::Animator* animator);

#pragma endregion
};

} // namespace editor

#endif