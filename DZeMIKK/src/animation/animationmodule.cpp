#include "animation/animationmodule.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/animator.h"
#include "spdlog/spdlog.h"

namespace dzemikk {
    void AnimationModule::update(float deltaTime) const {
        for (const auto& element : _animators) {
            element->update(deltaTime);
        }
    }
    void AnimationModule::Initialize() {
        ComponentRegistry::get().getComponents<Animator>(_animators);
        spdlog::info("Animator count: {}", _animators.size());
        //TODO: Get all animators components from component registry
    }
    void AnimationModule::UnInitialize() {
        _animators.clear();
        //TODO: Clear _animators vector
    }
} // namespace dzemikk
