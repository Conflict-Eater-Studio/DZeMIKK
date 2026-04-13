#include "animation/animationmodule.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/animator.h"
#include "spdlog/spdlog.h"

namespace dzemikk {
    void AnimationModule::update(float deltaTime) const {
        std::vector<Animator*> out;
        ComponentRegistry::get().getComponents<Animator>(out);
        for (const auto& element : out) {
            element->update(deltaTime);
        }
    }
    void AnimationModule::Initialize() {

        //TODO: Get all animators components from component registry
    }
    void AnimationModule::UnInitialize() {
        _animators.clear();
        //TODO: Clear _animators vector
    }
} // namespace dzemikk
