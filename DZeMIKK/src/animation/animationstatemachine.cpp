#include "animation/animationstatemachine.h"

#include "animation/animationstate.h"
#include "animation/transition.h"
#include "assetManager/assetmanager.h"
#include "spdlog/spdlog.h"

namespace dzemikk {
    AnimationState* AnimationStateMachine::addState() {
        std::unique_ptr<AnimationState> state = std::make_unique<AnimationState>();
        state->setName("NewState" + _states.size());
        _states.emplace(state->getName(), std::move(state));
        return state.get();
    }

    AnimationState* AnimationStateMachine::addState(const std::string& name) {
        std::unique_ptr<AnimationState> state = std::make_unique<AnimationState>(name);
        AnimationState* ptr = state.get();
        _states.emplace(state->getName(), std::move(state));
        return ptr;
    }
    AnimationState* AnimationStateMachine::getState(const std::string& name) const {
        auto it = _states.find(name);
        if (it != _states.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    const std::unordered_map<std::string, std::unique_ptr<AnimationState>>&
    AnimationStateMachine::getStates() const {
        return _states;
    }
    } // namespace dzemikk