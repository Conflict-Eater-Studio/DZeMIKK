#include "ecs/components/animator.h"

#include "animation/animationclip.h"
namespace dzemikk {

    void Animator::update(float deltaTime) {
        _stateMachine->update(deltaTime);
        AnimationClip* _currentClip = _stateMachine->getCurrentState()->clip;
        if (_currentClip == nullptr) return;
        _time += deltaTime;
        float timeInTicks = _time * _currentClip->ticksPerSecond;
        timeInTicks = fmod(timeInTicks, _currentClip->durationInTicks);
        //_currentClip->sample(timeInTicks, pose)
    }

    void Animator::play(const std::string& stateName) {
        _time = 0.0f;
    }

}


