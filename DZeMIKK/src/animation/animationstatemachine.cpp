#include "animation/animationstatemachine.h"
#include "animation/animationstate.h"
#include "animation/transition.h"
namespace dzemikk {
    void AnimationStateMachine::update(float dt) {
        if (_currentState == nullptr) return;
        if (_currentState->transitions.empty()) return;
        
        for (auto element : _currentState->transitions) {
            if (element.condition) {
                _currentState = _states.at(element.targetState);
            }
        }
    }
    AnimationState* AnimationStateMachine::getCurrentState() const {
        return _currentState;
    }
}
