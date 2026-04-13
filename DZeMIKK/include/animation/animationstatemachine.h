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
        AnimationStateMachine(const AnimationStateMachine&) = delete;
        AnimationStateMachine& operator=(const AnimationStateMachine&) = delete;

        void update(float deltaTime);

        [[nodiscard]] AnimationState* getCurrentState() const;

        void addState(std::unique_ptr<AnimationState> state);
        void setState(const std::string& stateName);
    private:
        std::unordered_map<std::string, std::unique_ptr<AnimationState>> _states;
        AnimationState* _currentState = nullptr;
    };
}
#endif
