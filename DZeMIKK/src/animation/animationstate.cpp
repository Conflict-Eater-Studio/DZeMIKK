#include <utility>
#include "animation/animationstate.h"

namespace dzemikk {
AnimationState::AnimationState() : _name("NewState") {}
AnimationState::AnimationState(const std::string& name) : _name(name) {}
const std::string& AnimationState::getName() const noexcept {
        return _name;
    }
    void AnimationState::setName(const std::string& name) {
        _name = name;
    }
    AnimationClip* AnimationState::getClip() const noexcept {
        return _clip;
    }
    void AnimationState::setClip(AnimationClip* clip) {
        _clip = clip;
    }
    const std::vector<Transition>& AnimationState::getTransitions() const noexcept {
        return _transitions;
    }
    void AnimationState::addTransition(const Transition& transition) {
        _transitions.push_back(transition);
    }
    void AnimationState::update(float deltaTime) {

    }

    } // namespace dzemikk
