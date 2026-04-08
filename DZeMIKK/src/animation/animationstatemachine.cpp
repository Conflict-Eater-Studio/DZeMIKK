#include "animation/animationstatemachine.h"

#include "animation/animationstate.h"
#include "animation/transition.h"
#include "spdlog/spdlog.h"
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
    void AnimationStateMachine::addState(AnimationState* state) {
        if (state == nullptr) return;

        _states[state->getName()] = state;
        if (_currentState == nullptr) {
            _currentState = state;
        }
    }
    void AnimationStateMachine::setState(const std::string& stateName) {
        auto it = _states.find(stateName);
        if (it == _states.end()) {
            spdlog::error("State {} not found", stateName);
            return;
        }
        if (it->second == nullptr) {
            spdlog::error("State {} is nullptr", stateName);
            return;
        }

        _currentState = it->second;
    }
} // namespace dzemikk
