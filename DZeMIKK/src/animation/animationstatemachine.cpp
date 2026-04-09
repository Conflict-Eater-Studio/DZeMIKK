#include "animation/animationstatemachine.h"

#include "animation/animationstate.h"
#include "animation/transition.h"
#include "spdlog/spdlog.h"
#include "animation/animationclip.h"

namespace dzemikk {
    void AnimationStateMachine::update(float deltaTime) {
        if (_currentState == nullptr) return;
        if (_currentState->getTransitions().empty()) return;

        for (auto element : _currentState->getTransitions()) {
            if (element.condition) {
                _currentState = _states.at(element.targetState).get();
            }
        }
    }
    AnimationState* AnimationStateMachine::getCurrentState() const {
        return _currentState;
    }

    void AnimationStateMachine::addState(std::unique_ptr<AnimationState> state) {
        if (!state) return;
        auto name = state->getName();
        _currentState = _currentState ? _currentState : state.get();
        _states.emplace(std::move(name), std::move(state));
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

        _currentState = it->second.get();
    }
} // namespace dzemikk
