#pragma once
#ifndef DZEMIKK_ANIMATOR_H
#define DZEMIKK_ANIMATOR_H

#include "animation/animationstate.h"
#include "animation/rootMotion.h"
#include "assetManager/assetHandle.h"
#include "ecs/component.h"
#include "nlohmann/json.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
namespace dzemikk {
class AnimationState;
class ComponentSerializerRegistry;
struct Condition;
class AnimationStateMachine;
class Skeleton;
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
        void setFloat(const std::string& name, float value);
        /**
         * @brief Sets a boolean parameter.
         *
         * @param name Parameter name.
         * @param value Boolean value to assign.
         */
        void setBool(const std::string& name, bool value);
        /**
         * @brief Sets an integer parameter.
         *
         * @param name Parameter name.
         * @param value Integer value to assign.
         */
        void setInt(const std::string& name, int value);
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
        [[nodiscard]] AnimationState* getCurrentState() const noexcept;
        [[nodiscard]] float getFloat(const std::string& name) const;
        [[nodiscard]] bool getBool(const std::string& name) const;
        [[nodiscard]] int getInt(const std::string& name) const;
        [[nodiscard]] float getCurrentTime() const;
        [[nodiscard]] std::unordered_map<std::string, float> getFloatParams() const;
        [[nodiscard]] std::unordered_map<std::string, bool> getBoolParams() const;
        [[nodiscard]] std::unordered_map<std::string, int> getIntParams() const;

        bool evaluate(const Condition& c) const;

        std::string typeName() const override;

        void setApplyRootMotion(bool apply);
        [[nodiscard]] bool getApplyRootMotion() const;
        void setRootMotionMode(RootMotionMode mode);
        [[nodiscard]] RootMotionMode getRootMotionMode() const;
        void setRootMotionBoneName(const std::string& boneName);
        [[nodiscard]] const std::string& getRootMotionBoneName() const;
        void setSkeleton(Skeleton* skeleton);
        [[nodiscard]] Skeleton* getSkeleton() const;

      private:
        std::shared_ptr<AnimationStateMachine> _stateMachine = nullptr;
        std::unordered_map<std::string, float> _floatParams;
        std::unordered_map<std::string, bool>  _boolParams;
        std::unordered_map<std::string, int>   _intParams;
        AnimationState* _currentState = nullptr;
        float _currentTime = 0.0f;

        bool _applyRootMotion = false;
        RootMotionMode _rootMotionMode = RootMotionMode::Position;
        std::string _rootMotionBoneName;
        Skeleton* _skeleton = nullptr;
        glm::mat4 _prevRootWorldTransform{1.0f};
        bool _hasPrevRootTransform = false;

        RootMotionDelta extractRootMotionDelta();
        void applyRootMotionDelta(const RootMotionDelta& delta) const;
    };

} // namespace dzemikk

#endif