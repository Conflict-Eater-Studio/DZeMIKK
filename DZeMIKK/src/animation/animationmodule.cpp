#include "animation/animationmodule.h"

#include "spdlog/spdlog.h"

namespace dzemikk {
    void AnimationModule::update(float deltaTime) const {
        for (const auto& element : _animators) {
            element->update(deltaTime);
        }
    }
    void AnimationModule::Initialize() {
        //TODO: Get all animators components from component registry
    }
    void AnimationModule::UnInitialize() {
        //TODO: Clear _animators vector
    }
} // namespace dzemikk
