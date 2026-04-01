#include "animation/animationstatemachine.h"
#include "animation/animationstate.h"
#include "animation/transition.h"
namespace dzemikk {
    void AnimationStateMachine::update(float deltaTime) {
        if (_currentState == nullptr) return;
        if (_currentState->getTransitions().empty()) return;

        for (auto element : _currentState->getTransitions()) {
            if (element.condition) {
                _currentState = _states.at(element.targetState);
            }
        }
    }
    AnimationState* AnimationStateMachine::getCurrentState() const {
        return _currentState;
    }
    void AnimationStateMachine::addState(AnimationState* state) const {
    }
    void AnimationStateMachine::setState(const std::string& stateName) {
        _currentState = _states.at(stateName);
    }
} // namespace dzemikk
