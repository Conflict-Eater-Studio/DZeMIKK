#include "animation/animationmodule.h"

#include "../../include/ecs/components/animator.h"
#include "ecs/componentRegistry.h"
#include "spdlog/spdlog.h"

namespace dzemikk {
    void AnimationModule::update(float deltaTime)  {
        for (const auto& element : _animators) {
            element->update(deltaTime);
        }
    }
    void AnimationModule::registerAnimator(Animator* animator) {
        _animators.push_back(animator);
    }
    void AnimationModule::unregisterAnimator(const Animator* animator) {
        auto it = std::find(_animators.begin(), _animators.end(), animator);
        if (it != _animators.end()) {
            *it = _animators.back();
            _animators.pop_back();
        }
    }
    void AnimationModule::initialize() {

        //TODO: Get all animators components from component registry
    }
    void AnimationModule::uninitialize() {
        _animators.clear();
        //TODO: Clear _animators vector
    }
} // namespace dzemikk
