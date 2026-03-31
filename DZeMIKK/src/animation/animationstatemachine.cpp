#include "animation/animationstatemachine.h"
namespace dzemikk {
    void AnimationStateMachine::update(float dt) {
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
