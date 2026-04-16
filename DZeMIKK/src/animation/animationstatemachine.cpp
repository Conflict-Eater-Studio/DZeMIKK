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

    AnimationState* AnimationStateMachine::addState() {
        std::unique_ptr<AnimationState> state = std::make_unique<AnimationState>();
        state->setName("NewState" + std::to_string(_statesCount++));

        AnimationState* ptr = state.get();
        if (_currentState == nullptr) {
            _currentState = ptr;
        }
        _states.emplace(state->getName(), std::move(state));
        return ptr;
    }
AnimationState* AnimationStateMachine::addState(std::string name) {
        std::unique_ptr<AnimationState> state = std::make_unique<AnimationState>(name);
       _statesCount++;

        AnimationState* ptr = state.get();
        if (_currentState == nullptr) {
            _currentState = ptr;
        }

        _states.emplace(state->getName(), std::move(state));
        return ptr;
    }

    void AnimationStateMachine::setState(const std::string& stateName) {
        auto it = _states.at(stateName).get();

        if (it == _currentState) return;
        if (it == nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::error("State {} is nullptr", stateName);
#endif
            return;
        }

        _currentState = it;
    }
}
