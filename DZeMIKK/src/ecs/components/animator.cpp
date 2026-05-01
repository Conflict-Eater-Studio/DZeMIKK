#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "ecs/components/animator.h"
#include "animation/animationclip.h"
#include "animation/animationstate.h"
#include "animation/animationstatemachine.h"

namespace dzemikk {
    void Animator::update(float deltaTime) {
        if (_stateMachine == nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("[Animator] Animator has no state machine!");
#endif
            return;
        }
        if (_currentState == nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("[Animator] Animator has no current state!");
#endif
            return;
        }
        AnimationClip* clip = _currentState->getClip();
        if (clip == nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("[Animator] Current state has no clip!");
#endif
            return;
        }

        std::vector<Transition> transitions = _currentState->getTransitions();
        if (transitions.empty()) return;

        for (auto transition: transitions) {
            if (evaluate(transition.condition)) {
                _currentState = _stateMachine->getState(transition.targetState);
            };
        }
    }

    void Animator::play(std::string& stateName) {
        if (_stateMachine == nullptr) {
            return;
        }

        _currentState = _stateMachine->getState(stateName);
        if (_currentState == nullptr) {
            return;
        }

        _currentTime = 0.0f;

    }
    void Animator::setFloat(const std::string& name, float value) {
            _parameters[name] = value;
    }

    void Animator::setBool(const std::string& name, bool value) {
            _parameters[name] = value;
    }

    void Animator::setInt(const std::string& name, int value) {
            _parameters[name] = value;
    }
    void Animator::setStateMachine(const std::shared_ptr<AnimationStateMachine>& stateMachine) {
        _stateMachine = stateMachine;
    }
    std::shared_ptr<AnimationStateMachine> Animator::getStateMachine() const noexcept {
        return _stateMachine;
    }
    AnimationState* Animator::getCurrentState() const noexcept {
        return _currentState;
    }
    float Animator::getFloat(const std::string& name) const {
            auto it = _parameters.find(std::string(name));
            if (it != _parameters.end()) {
                if (auto val = std::get_if<float>(&it->second))
                    return *val;
            }
            return 0.0f;
        }

    bool Animator::getBool(const std::string& name) const {
            auto it = _parameters.find(std::string(name));
            if (it != _parameters.end()) {
                if (auto val = std::get_if<bool>(&it->second)) {
                    return *val;
                }
            }
            return false;
        }

    int Animator::getInt(const std::string& name) const {
            auto it = _parameters.find(std::string(name));
            if (it != _parameters.end()) {
                if (auto val = std::get_if<int>(&it->second)) {
                    return *val;
                }
            }
            return 0;
    }
    float Animator::getCurrentTime() const {
        return _currentTime;
    }

    std::string Animator::typeName() const {
        return "Animator";
    }
    bool Animator::evaluate(const Condition& c) const {
        float param = getFloat(c.parameter);

        switch (c.op) {
            case Greater: return param > c.value;
            case Less: return param < c.value;
            case Equal: return param == c.value;
            case NotEqual: return param != c.value;
            default:
                return false;
            }

        return false;
    }
}


