#include "animation/animationstate.h"

#include "animation/animationclip.h"

#include <utility>

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
void AnimationState::resetTime() noexcept {
    _currentTime = 0.0f;
}
void AnimationState::update(float deltaTime) {
    _currentTime += deltaTime;

    if (_clip == nullptr) {
        return;
    }

    _clip->apply(_currentTime);
}
}