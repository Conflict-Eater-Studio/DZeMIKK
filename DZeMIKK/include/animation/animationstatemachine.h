#pragma once
#ifndef DZEMIKK_ANIMATIONSTATEMACHINE_H
#define DZEMIKK_ANIMATIONSTATEMACHINE_H

#include <map>
#include <string>
namespace dzemikk {
    class AnimationState;
    class AnimationStateMachine {
    public:
        void update(float dt);
        AnimationState* getCurrentState() const;
    private:
        std::map<std::string, AnimationState*> _states;
        AnimationState* _currentState;
    };
}
#endif
