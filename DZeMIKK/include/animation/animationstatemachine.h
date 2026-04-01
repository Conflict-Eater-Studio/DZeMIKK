#pragma once
#ifndef DZEMIKK_ANIMATIONSTATEMACHINE_H
#define DZEMIKK_ANIMATIONSTATEMACHINE_H

#include <map>
#include <string>
namespace dzemikk {
    class AnimationState;
    class AnimationStateMachine {
    public:
        AnimationStateMachine() = default;
        ~AnimationStateMachine() = default;
        void update(float deltaTime);
        [[nodiscard]] AnimationState* getCurrentState() const;
        void addState(AnimationState* state) const;
        void setState(const std::string& stateName);
    private:
        std::map<std::string, AnimationState*> _states;
        AnimationState* _currentState = nullptr;
    };
}
#endif
