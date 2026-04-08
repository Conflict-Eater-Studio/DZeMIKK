#include "ecs/components/animator.h"

#include "animation/animationclip.h"
#include "animation/animationstate.h"
#include "ecs/components/transform.h"

#include <spdlog/spdlog.h>

namespace dzemikk {
    void Animator::update(float deltaTime) {
        if (_stateMachine == nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("Animator has no state machine!");
#endif
            return;
        }
        _stateMachine->update(deltaTime);
        AnimationState* _state = _stateMachine->getCurrentState();
        if (_state == nullptr) {
            spdlog::warn("AnimationStateMachine has no states!");
            return;
        }
        AnimationClip* _currentClip = _state->getClip();
        if (_currentClip == nullptr){
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("State {} has no clip!", _state->getName());
#endif
            return;
        };

        _time += deltaTime;

        float time = _time * _currentClip->getFramerate(); // seconds in animation clip timeline
        float lengthInSeconds = _currentClip->getLength() / _currentClip->getFramerate();
        float keyframe = fmod(time, lengthInSeconds);

        _currentClip->sample(keyframe);
    }

    void Animator::play(const std::string& stateName) {
        _time = 0.0f;
        _stateMachine->setState(stateName);
    }
    void Animator::setStateMachine(AnimationStateMachine* stateMachine) {
        _stateMachine = stateMachine;
    }
    AnimationStateMachine* Animator::getStateMachine() const {
        return _stateMachine;
    }

    }


