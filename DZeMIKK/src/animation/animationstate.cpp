#include <utility>
#include "animation/animationstate.h"

namespace dzemikk {
    AnimationState::AnimationState(std::string name) : _name(std::move(name)) {}
    AnimationState::~AnimationState() {}
    const std::string& AnimationState::getName() const {
        return _name;
    }
    void AnimationState::setName(const std::string& name) {
        _name = name;
    }
    AnimationClip* AnimationState::getClip() const {
        return _clip;
    }
    void AnimationState::setClip(AnimationClip* clip) {
        _clip = clip;
    }
    const std::vector<Transition>& AnimationState::getTransitions() const {
        return _transitions;
    }
    void AnimationState::addTransition(const Transition& transition) {
        _transitions.push_back(transition);
    }
    void AnimationState::update(float deltaTime) {

    }

    } // namespace dzemikk
