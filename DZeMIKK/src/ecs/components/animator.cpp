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
        _stateMachine->update(deltaTime);
    }
    void Animator::play(const std::string& stateName) const {
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
    std::string Animator::typeName() const {
        return "Animator";
    }
}


