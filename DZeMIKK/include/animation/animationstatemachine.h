#pragma once
#ifndef DZEMIKK_ANIMATIONSTATEMACHINE_H
#define DZEMIKK_ANIMATIONSTATEMACHINE_H

#include <memory>
#include <string>
#include <unordered_map>

namespace dzemikk {
class AnimationClip;
class AnimationState;
    class AnimationStateMachine {
    public:
        AnimationStateMachine() = default;
        ~AnimationStateMachine() = default;

        void update(float deltaTime);

        [[nodiscard]] AnimationState* getCurrentState() const;
        void addState(std::unique_ptr<AnimationState> state);
        void addState(AnimationState stateName, AnimationClip clip);

        void setState(const std::string& stateName);
    private:
        std::unordered_map<std::string, std::unique_ptr<AnimationState>> _states;
        AnimationState* _currentState = nullptr;
    };
}
#endif
