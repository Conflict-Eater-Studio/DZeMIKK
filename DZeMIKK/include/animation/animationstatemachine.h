#pragma once
#ifndef DZEMIKK_ANIMATIONSTATEMACHINE_H
#define DZEMIKK_ANIMATIONSTATEMACHINE_H
#include "animationstate.h"

#include <map>
namespace dzemikk {
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
