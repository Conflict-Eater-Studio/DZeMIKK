#include <spdlog/spdlog.h>

#include "ecs/components/animator.h"
#include "animation/animationclip.h"
#include "animation/animationstate.h"

namespace dzemikk {
    void Animator::update(float deltaTime) {
        _stateMachine->update(deltaTime);
        AnimationClip* _currentClip = _stateMachine->getCurrentState()->clip;
        if (_currentClip == nullptr) return;
        _time += deltaTime;
        float timeInTicks = _time * _currentClip->ticksPerSecond;
        timeInTicks = fmod(timeInTicks, _currentClip->durationInTicks);
#if DZEMIKK_DEV_TOOLS
        spdlog::info("Animation time: {}", timeInTicks);
#endif
        //_currentClip->sample(timeInTicks, pose)
    }

    void Animator::play(const std::string& stateName) {
        _time = 0.0f;
    }
    void Animator::setStateMachine(AnimationStateMachine* stateMachine) {
        _stateMachine = stateMachine;
    }
    AnimationStateMachine* Animator::getStateMachine() const {
        return _stateMachine;
    }

    }


