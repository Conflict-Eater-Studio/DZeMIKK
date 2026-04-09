#pragma once
#ifndef DZEMIKK_ANIMATOR_H
#define DZEMIKK_ANIMATOR_H

#include "ecs/component.h"

#include <memory>
#include <string>
#include <string_view>

namespace dzemikk {
    class AnimationStateMachine;
    /**
     * @brief Handles animation playback and state transitions for an entity.
     *
     * The Animator component is responsible for:
     * - Updating the current animation state over time
     * - Managing animation parameters (float, bool, int)
     * - Driving transitions inside the AnimationStateMachine
     *
     * Each Animator instance represents a runtime animation controller
     * for a single entity. It holds its own state (e.g. time, parameters),
     * while the AnimationStateMachine defines shared animation logic.
     */
    class Animator : public Component {
    public:
        /**
         * @brief Updates the animator and its state machine.
         *
         * Advances the internal time and evaluates the current animation state.
         * Also processes transitions based on parameters.
         *
         * @param deltaTime Time elapsed since last frame (in seconds).
         */
        void update(float deltaTime);

        /**
         * @brief Immediately switches to a specific animation state.
         *
         * This bypasses transition conditions and forces the state change.
         *
         * @param stateName Name of the target state.
         */
        void play(const std::string& stateName);

        /**
         * @brief Sets a float parameter.
         *
         * Parameters are used by transitions to determine state changes.
         *
         * @param name Parameter name.
         * @param value Float value to assign.
         */
        void setFloat(std::string_view name, float value);

        /**
         * @brief Sets a boolean parameter.
         *
         * @param name Parameter name.
         * @param value Boolean value to assign.
         */
        void setBool(std::string_view name, bool value);

        /**
         * @brief Sets an integer parameter.
         *
         * @param name Parameter name.
         * @param value Integer value to assign.
         */
        void setInt(std::string_view name, int value);

        /**
         * @brief Assigns an animation state machine to the animator.
         *
         * The Animator shares ownership of the state machine.
         * The state machine defines animation states and transitions.
         *
         * @param stateMachine Shared pointer to the state machine.
         */
        void setStateMachine(const std::shared_ptr<AnimationStateMachine>& stateMachine);

        /**
         * @brief Returns the current animation state machine.
         *
         * @return Shared pointer to the state machine.
         */
        [[nodiscard]] std::shared_ptr<AnimationStateMachine> getStateMachine() const noexcept;

    private:
        /// Shared animation state machine defining states and transitions.
        std::shared_ptr<AnimationStateMachine> _stateMachine = nullptr;

        /// Current playback time within the active animation (in seconds).
        float _currentTime = 0.0f;
    };

} // namespace dzemikk

#endif