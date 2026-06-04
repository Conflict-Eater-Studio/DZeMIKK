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

        clip->apply(_currentTime);
        _currentTime += deltaTime;

        std::vector<Transition> transitions = _currentState->getTransitions();
        if (transitions.empty()) return;

        for (auto transition: transitions) {
            if (evaluate(transition.condition)) {
                play(transition.targetState);
                break;
            };
        }
    }

    void Animator::play(const std::string& stateName) {
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
            _floatParams[name] = value;
    }

    void Animator::setBool(const std::string& name, bool value) {
            _boolParams[name] = value;
    }

    void Animator::setInt(const std::string& name, int value) {
            _intParams[name] = value;
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
        auto it = _floatParams.find(name);
        return (it != _floatParams.end()) ? it->second : 0.0f;
    }

    bool Animator::getBool(const std::string& name) const {
        auto it = _boolParams.find(name);
        return (it != _boolParams.end()) ? it->second : false;
    }

    int Animator::getInt(const std::string& name) const {
        auto it = _intParams.find(name);
        return (it != _intParams.end()) ? it->second : 0;
    }
    float Animator::getCurrentTime() const {
        return _currentTime;
    }
    std::unordered_map<std::string, float> Animator::getFloatParams() const {
        return _floatParams;
    }
    std::unordered_map<std::string, bool> Animator::getBoolParams() const {
        return _boolParams;
    }
    std::unordered_map<std::string, int> Animator::getIntParams() const {
        return _intParams;
    }

    std::string Animator::typeName() const {
        return "Animator";
    }
    bool Animator::evaluate(const Condition& c) const {

        switch (c.type) {

        case ParamType::Float: {
            float param = getFloat(c.parameter);

            switch (c.op) {
            case Greater:
                return param > c.value;
            case Less:
                return param < c.value;
            case Equal:
                return param == c.value;
            case NotEqual:
                return param != c.value;
            }
            break;
        }

        case ParamType::Int: {
            int param = getInt(c.parameter);
            int v = static_cast<int>(c.value);

            switch (c.op) {
            case Greater:
                return param > v;
            case Less:
                return param < v;
            case Equal:
                return param == v;
            case NotEqual:
                return param != v;
            }
            break;
        }

        case ParamType::Bool: {
            bool param = getBool(c.parameter);
            bool v = (c.value != 0.0f);

            switch (c.op) {
            case Equal:
                return param == v;
            case NotEqual:
                return param != v;
            default:
                return false; 
            }
        }
        }

        return false;
    }
    }