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

        _currentTime += deltaTime;

        _currentClip->sample(_currentTime);
    }
    void Animator::play(const std::string& stateName) {
        _currentTime = 0.0f;
        _stateMachine->setState(stateName);
    }
    void Animator::setFloat(std::string_view name, float value) {

    }
    void Animator::setBool(std::string_view name, bool value) {

    }
    void Animator::setInt(std::string_view name, int value) {

    }
    void Animator::setStateMachine(const std::shared_ptr<AnimationStateMachine>& stateMachine) {
        _stateMachine = stateMachine;
    }
    std::shared_ptr<AnimationStateMachine> Animator::getStateMachine() const noexcept {
        return _stateMachine;
    }
}


